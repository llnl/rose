#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include <Rose/BinaryAnalysis/ByteCode/Analysis.h>
#include <Rose/BinaryAnalysis/ByteCode/Jvm.h>

#include <Rose/BinaryAnalysis/Address.h>
#include <Rose/BinaryAnalysis/Architecture/Base.h>
#include <Rose/BinaryAnalysis/Partitioner2/BasicBlock.h>
#include <Rose/BinaryAnalysis/Partitioner2/Partitioner.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/TraceSemantics.h>

#include <SgAsmInstruction.h>
#include <SgAsmInstructionList.h>
#include <Cxx_GrammarDowncast.h>

#include <iostream>

using namespace Rose::BinaryAnalysis::Partitioner2;
using PoolEntry = SgAsmJvmConstantPoolEntry;
using AddressSegment = Sawyer::Container::AddressSegment<Rose::BinaryAnalysis::Address,uint8_t>;
using Rose::Diagnostics::DEBUG;
using Rose::Diagnostics::INFO;
using Rose::StringUtility::addrToString;

namespace Rose {
namespace BinaryAnalysis {
namespace ByteCode {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Field::Field() = default;
Field::~Field() = default;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Method
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Method::Method(std::string name, Address va) : name_{std::move(name)}, address_{va} {}
Method::~Method() = default;

const std::string&
Method::name() const {
    return name_;
}

Address
Method::address() const {
    return address_;
}

Class*
Method::declaringClass() const {
    return class_;
}

void
Method::declaringClass(Class *declaringClass) {
    ASSERT_not_null(declaringClass);
    class_ = declaringClass;
}

void
Method::finalize() {
    instructionMap_.clear();

    for (SgAsmInstruction* insn : instructions()->get_instructions()) {
        ASSERT_not_null(insn);

        const bool inserted = instructionMap_.emplace(insn->get_address(), insn).second;
        ASSERT_require2(inserted, "duplicate instruction address in method");
    }
}

SgAsmInstruction*
Method::instructionAt(Address va) const {
    const auto found = instructionMap_.find(va);
    return found == instructionMap_.end()
        ? nullptr
        : found->second;
}

const std::vector<BasicBlock::Ptr>&
Method::blocks() const {
    return blocks_;
}

void
Method::append(BasicBlock::Ptr bb) {
    blocks_.push_back(bb);
}

std::set<Address>
Method::targets() const {
    std::set<Address> retval{};
    for (auto insn : instructions()->get_instructions()) {
        bool complete = true;
        auto successors = insn->architecture()->getSuccessors(insn, complete /*out*/);
        for (auto successor : successors.values()) {
            mlog[DEBUG] << "... Method::targets():adding successor target va: " << addrToString(successor)<< "\n";
            retval.insert(successor);
        }
    }
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Interface::Interface() {}
Interface::~Interface() = default;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Attribute
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Attribute::Attribute() {}
Attribute::~Attribute() = default;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Class::Class(std::string name, Address va, NamespacePtr ns) : name_{std::move(name)}, address_{va}, namespace_{ns} {}
Class::~Class() {}

const std::string&
Class::name() const {
    return name_;
}

const std::string&
Class::baseClassName() const {
    return baseClassName_;
}

Address
Class::address() const {
    return address_;
}

const std::vector<Interface::Ptr>&
Class::interfaces() const {
    return interfaces_;
}

const std::vector<Field::Ptr>&
Class::fields() const {
    return fields_;
}

const std::vector<Method::Ptr>&
Class::methods() const {
    return methods_;
}

const std::vector<Attribute::Ptr>&
Class::attributes() const {
    return attributes_;
}

const std::vector<std::string>&
Class::strings() {
    return strings_;
}

void
Class::finalize() {
    for (const Method::Ptr &method: methods_)
        method->finalize();
}

void
Class::partition(const PartitionerPtr &partitioner, std::map<std::string,Address> &discoveredFunctions,
                 const Progress::Ptr &progress) {
    const size_t nBits = 64;
    const size_t nMethods = methods().size();
    size_t nMethodsHandled = 0;
    ProgressTask methodsTask(progress, "methods");

    if (progress) {
        if (nMethods > 0) {
            progress->update(Progress::Report("", 0.0, (double)nMethods));
        } else {
            progress->update(Progress::Report("", 1.0, 1.0));
        }
    }

    for (auto method : methods()) {
        Address va{0};
        FunctionPtr function{};
        BasicBlockPtr block{};
        bool needNewBlock{true};
        bool insertFallthroughSuccessors{true};
        Address seenVa{0};

        // Provide the ByteCode::Method with access to its declaring ByteCode::Class
        method->declaringClass(this);

        // Annotate the instructions
        method->annotate();

        auto instructions = method->instructions()->get_instructions();
        if (!instructions.empty()) {
            // The address of the Partitioner2::Function is the address of the first basic block
            va = instructions.front()->get_address();
        } else {
            // A Java interface has no instructions, use the class address instead
            ASSERT_require2(false, "Need unique method address");
        }

        // Determine if this method/function has been seen before (e.g., ".ctor" of parent class)
        if (partitioner->placeholderExists(va)) {
            ++nMethodsHandled;
            if (progress)
                progress->update(Progress::Report("", (double)nMethodsHandled, (double)nMethods));
            continue;
        }

        // Create the (Partitioner2) function
        std::string functionName = name() + typeSeparator() + method->name();

        // Include method descriptor for JVM methods to distinguish overloaded methods (including bridge methods)
        if (auto jvmMethod = ByteCode::JvmMethod::promote(method)) {
            functionName += jvmMethod->descriptor();
        }

        function = Partitioner2::Function::instance(va, functionName);

        // Add newly discovered function to the list
        if (discoveredFunctions.find(functionName) == discoveredFunctions.end()) {
            discoveredFunctions[functionName] = va;
        }
        else {
            // This occurs for a nested/inner class with a constructor having an input parameter
            // of a dynamic type (I think, for example, see T8_NestMembersAttribute.java).
            // Also dotnet (see T1_Test1.cs).
            mlog[Diagnostics::DEBUG] << "Class::partition(): discovered duplicate function: " << functionName
                                      << " (old va=" << StringUtility::addrToString(discoveredFunctions[functionName])
                                      << ", new va=" << StringUtility::addrToString(va) << ")\n";
        }

        std::set<Address> targets = method->targets();

        for (auto astInsn : instructions) {
            // A copy of the instruction must be made if it is linked to ROSE's AST
            SgTreeCopy deep;
            SgAsmInstruction* insn = isSgAsmInstruction(astInsn->copy(deep));
            ASSERT_not_null(insn);
            ASSERT_require(insn != astInsn);
            ASSERT_require(insn->get_address() == astInsn->get_address());

            // A new block is needed if this instruction is a target of a branch and nonterminal
            va = insn->get_address();
            if (targets.find(va) != targets.end()) {
                // But a new block is not needed if this is the first instruction in the block
                if (block && !block->isEmpty() && va != block->address()) {
                    mlog[DEBUG] << "Splitting block after: " << addrToString(block->instructions().back()->get_address())
                                << " va: " << addrToString(va)
                                << " fallthrough: " << addrToString(block->fallthroughVa())
                                << " kind:" << insn->get_anyKind() << " :" << insn->get_mnemonic() << "\n";

                    // If the instruction doesn't have a branch target, add fall through successor
                    if (!partitioner->architecture()->branchTarget(block->instructions().back())) {
                        mlog[DEBUG] << "Adding successor fall-through edge from va: "
                                    << addrToString(block->instructions().back()->get_address())
                                    << " to: " << addrToString(block->fallthroughVa()) << "\n";
                        if (insertFallthroughSuccessors) {
                            block->insertSuccessor(block->fallthroughVa(), nBits, EdgeType::E_NORMAL, Confidence::PROVED);
                        } else {
                            insertFallthroughSuccessors = true;
                        }
                    }
                    needNewBlock = true;
                }
            }

            // If needed, create a new block and insert it into the function
            if (needNewBlock) {
                if (block && !block->isEmpty() && va != block->address()) {
                    // Attach old block only if its address differs from the new instruction's
                    partitioner->attachBasicBlock(block);
                }
                block = Partitioner2::BasicBlock::instance(va, partitioner);

                function->insertBasicBlock(va);
                method->append(block);
                needNewBlock = false;
            }

            if (block->address() == function->address() && function->address() != seenVa) {
//TODO: Save this for initializing frames
#if 0
                // Create and push the this pointer for a new frame
                // TODO: This should be done by initializeFrame
                // CIL needs ops to work with frames
                if (ops) {
                    seenVa = function->address();

                    std::string desc = "L";
                    desc += name() + ";";

                    auto this_ = ops->unspecified_(32);
                    ASSERT_require(this_->nBits() == 32);

                    this_->kind(InstructionSemantics::BaseSemantics::ValueKind::ObjectReference);
                    this_->typeDescriptor(desc);

                    ops->writeLocal(0, this_);
                }
#endif
            }

            // Warning: this instruction can't be linked into ROSE's AST (parent must be null)
            block->append(partitioner, insn);

            // If this instruction terminates the block, add successors and set block properties
            if (partitioner->architecture()->terminatesBasicBlock(insn)) {
                bool complete;
                auto successors = partitioner->architecture()->getSuccessors(insn, complete/*out*/);
                for (auto successor : successors.values()) {
                    mlog[DEBUG] << "Adding successor edge from va: "
                                << addrToString(va) << " to: " << addrToString(successor) << "\n";
                    block->insertSuccessor(successor, nBits, EdgeType::E_NORMAL, Confidence::PROVED);
                }

                // Set properties of the block
                SgAsmInstruction* last = block->instructions().back();
                if (partitioner->architecture()->isFunctionReturnFast(block->instructions())) {
                    block->isFunctionReturn(true);
                }
                else if (partitioner->architecture()->isFunctionCallFast(block->instructions(), nullptr, nullptr)) {
                    block->isFunctionCall(true);

                    // Fully resoved function name is stored in the comment of the call instruction
                    std::string callee = last->get_comment();

                    // Insert a partitioner function for a system call when first seen
                    if (discoveredFunctions.find(callee) == discoveredFunctions.end()) {
                        if (method->isSystemReserved(callee)) {
                            Address reservedVa = Container::nextSystemReservedVa();
                            auto reservedFunction = Partitioner2::Function::instance(reservedVa, callee);
                            auto reservedBlock = Partitioner2::BasicBlock::instance(reservedVa, partitioner);

                            reservedFunction->insertBasicBlock(reservedVa);
                            partitioner->attachBasicBlock(reservedBlock);
                            partitioner->attachFunction(reservedFunction);

                            // It's been discovered
                            discoveredFunctions[callee] = reservedVa;
                        }
                    }

                    if (discoveredFunctions.find(callee) != discoveredFunctions.end()) {
                        auto itr = discoveredFunctions.find(callee);
                        mlog[DEBUG] << "Adding call edge from va: " << addrToString(va) << " to: " << addrToString(itr->second)
                                    << " : " << callee << "\n";
                        block->insertSuccessor(itr->second, nBits, EdgeType::E_FUNCTION_CALL, Confidence::PROVED);
                    }
                    else {
                        mlog[DEBUG] << "Failed to find function, NOT adding call edge from va: "
                                    << addrToString(va) <<" to: " <<callee <<"\n";
                        // No fallthrough successor should be added to allow edge to indeterminate vertex to be created
                        insertFallthroughSuccessors = false;
                    }
                }
                needNewBlock = true;
            }
        }

        // If this is an interface an empty block will need to be created
        if (block == nullptr) {
            block = Partitioner2::BasicBlock::instance(va, partitioner);
            function->insertBasicBlock(va);
            method->append(block);
        }

        // Attach function return block and function to the partitioner
        partitioner->attachBasicBlock(block);
        partitioner->attachFunction(function);

        ++nMethodsHandled;
        if (progress)
            progress->update(Progress::Report("", (double)nMethodsHandled, (double)nMethods));
    }
}

void Class::digraph() const {
    std::ofstream dotFile;
    mlog[INFO] << "Opening dot file " + name() + ".dot\n";
    dotFile.open(name() + ".dot");
    dotFile << "digraph g {" << "\n";

    dotFile << "\n";
    dotFile << "  fontname=\"Helvetica,Arial,sans-serif\"\n";
    dotFile << "  node [fontname=\"Helvetica,Arial,sans-serif\"]\n";
    dotFile << "  edge [fontname=\"Helvetica,Arial,sans-serif\"]\n";
    dotFile << "  node [fontsize = \"16\" shape = \"ellipse\"]\n";
    dotFile << "  graph [rankdir = \"LR\"]\n\n";

    // Edge from class to each method
    for (auto method : methods()) {
        dotFile << "  " << name() << " -> \"" << method->name() << "\";\n";
    }
    dotFile << "\n";

    // Blocks for each method
    for (size_t midx = 0; midx < methods().size(); midx++) {
        auto method = methods()[midx];
        for (size_t bidx = 0; bidx < method->blocks().size(); bidx++) {
            auto block = method->blocks()[bidx];
            dotFile << "  block_" << midx << "_" << bidx << " [\n    label=\"";
            for (auto insn : block->instructions()) {
                dotFile << "<" << insn->get_address() << ">" << insn->get_mnemonic() << ":" << insn->get_address();
                if (insn != block->instructions().back()) dotFile << "|";
            }
            dotFile << "\"\n    shape = \"record\"\n  ];" << "\n";
        }
    }
    dotFile << "\n";

    for (size_t midx = 0; midx < methods().size(); midx++) {
        auto method = methods()[midx];

        std::map<Address,int> vaToBlock{};
        for (size_t bidx = 0; bidx < method->blocks().size(); bidx++) {
            auto block = method->blocks()[bidx];
            for (auto insn : block->instructions()) {
                vaToBlock.emplace(insn->get_address(),bidx);
            }
        }

        // Edge from method to first block
        if (method->blocks().size() > 0) {
            auto blockHead = method->blocks()[0];
            dotFile << "  \"" << method->name() << "\" -> "
                    << "block_" << midx << "_0:" << blockHead->instructions()[0]->get_address() << "\n";
        }

        for (size_t bidx = 0; bidx < method->blocks().size(); bidx++) {
            auto block = method->blocks()[bidx];
            auto tail = block->instructions().back();
            // Successors edges from the block
            if (block->successors().isCached()) {
                for (auto successor : block->successors().get()) {
                    if (auto targetVa = successor.expr()->toUnsigned()) {
                        Address va = targetVa.get();
                        dotFile << "  block_" << midx << "_" << bidx << ":" << tail->get_address()
                                << " -> block_" << midx << "_" << vaToBlock[va] << ":" << va << "\n";
                    }
                }
            }
        }
    }
    dotFile << "}\n";
    dotFile.close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Namespace::Namespace() {}
Namespace::~Namespace() {}

Namespace::Ptr
Namespace::instance() {
    return Ptr(new Namespace());
}

std::string
Namespace::name() const {
    return "NoNamespaceName";
}

const std::vector<ByteCode::Class::Ptr>&
Namespace::classes() const {
    return classes_;
}

void
Namespace::append(Class::Ptr ptr) {
    classes_.push_back(ptr);
}

void
Namespace::partition(const PartitionerPtr& partitioner, std::map<std::string,Address> &discoveredFunctions) {
    for (auto cls: classes()) {
        cls->partition(partitioner, discoveredFunctions);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Container
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Address
Container::nextSystemReservedVa_{static_cast<Address>(-1)};

Container::Container() {}
Container::~Container() {}

Address
Container::nextSystemReservedVa() {
    Address va{nextSystemReservedVa_};
    nextSystemReservedVa_ -= 1024;
    return va;
}

const std::vector<Namespace::Ptr>&
Container::namespaces() const {
    return namespaces_;
}

void
Container::partition(const PartitionerPtr& partitioner) {
    // Both Cil and Jvm need call return edges to be added by Partitioner::attachBasicBlock()
    partitioner->autoAddCallReturnEdges(true);

    // Do the partitioning and keep track of functions discovered during the process
    std::map<std::string,Address> discoveredFunctions{};

    for (auto nmSpace: namespaces_) {
        nmSpace->partition(partitioner, discoveredFunctions);
    }

    // Attach empty functions as targets for invoke of system functions
    for (auto discovered: discoveredFunctions) {
        Address va = discovered.second;
        std::string name = discovered.first;

        // Create and attach system functions if placeholder doesn't exist already
        if (isSystemReserved(name)) {
            if (!partitioner->placeholderExists(va)) {
                auto function = Partitioner2::Function::instance(va, name);
                auto block = Partitioner2::BasicBlock::instance(va, partitioner);

                function->insertBasicBlock(va);
                partitioner->attachBasicBlock(block);
                partitioner->attachFunction(function);
            }
        }
    }
    // partitioner->dumpCfg(std::cout, "Worker:", true, false);
}

} // namespace
} // namespace
} // namespace

#endif
