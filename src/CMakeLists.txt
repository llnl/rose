set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ddisc_union=union")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ddisc_union=union")

add_executable(rose-feature-tests featureTests.C)

########################################################################################################################
# ROSETTA
########################################################################################################################

# Header files needed to build ROSETTA
include_directories(
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/src/roseSupport
  ${CMAKE_SOURCE_DIR}/src/roseExtensions/failSafe
  ${CMAKE_SOURCE_DIR}/src/midend/programTransformation/transformationTracking
  ${CMAKE_BINARY_DIR}/src/3rdPartyLibraries/fortran-parser
  ${JAVA_INCLUDE_PATH}
  ${JAVA_INCLUDE_PATH2}
  ${CMAKE_SOURCE_DIR}/src/frontend/ECJ_ROSE_Connection
  ${CMAKE_SOURCE_DIR}/src/midend/astMatching
  ${CMAKE_SOURCE_DIR}/src/util
  ${CMAKE_SOURCE_DIR}/src/frontend
  ${ROSE_INCLUDES})

# Output directory for ROSETTA-generated files
set(ROSETTA_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/frontend/SageIII)

# ROSETTA-generated class definition header files.
# Windows has problems executing commands with even relatively small command-lines, so we have to split this list of files
# into smaller lists. Windows command cannot be longer than 2k characters.
set(ROSETTA_GENERATED_DEFINITIONS_01
  ${ROSETTA_OUTPUT_DIRECTORY}/SgNode.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmAarch32Coprocessor.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmAarch32Instruction.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmAarch64AtOperand.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmAarch64BarrierOperand.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmAarch64CImmediateOperand.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmAarch64Instruction.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmAarch64PrefetchOperand.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmAarch64SysMoveOperand.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBasicString.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBinaryAdd.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBinaryAsr.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBinaryConcat.h
)
set(ROSETTA_GENERATED_DEFINITIONS_02
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBinaryDivide.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBinaryExpression.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBinaryLsl.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBinaryLsr.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBinaryMod.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBinaryMsl.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBinaryMultiply.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBinaryPostupdate.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBinaryPreupdate.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBinaryRor.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBinarySubtract.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmBlock.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmByteOrder.h
)
set(ROSETTA_GENERATED_DEFINITIONS_03
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilAssembly.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilAssemblyOS.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilAssemblyOSTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilAssemblyProcessor.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilAssemblyProcessorTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilAssemblyRef.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilAssemblyRefOS.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilAssemblyRefOSTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilAssemblyRefProcessor.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilAssemblyRefProcessorTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilAssemblyRefTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilAssemblyTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilClassLayout.h
)
set(ROSETTA_GENERATED_DEFINITIONS_04
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilClassLayoutTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilConstant.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilConstantTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilCustomAttribute.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilCustomAttributeTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilDataStream.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilDeclSecurity.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilDeclSecurityTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilErrorHeap.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilEvent.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilEventMap.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilEventMapTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilEventTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilExportedType.h
)
set(ROSETTA_GENERATED_DEFINITIONS_05
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilExportedTypeTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilField.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilFieldLayout.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilFieldLayoutTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilFieldMarshal.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilFieldMarshalTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilFieldRVA.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilFieldRVATable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilFieldTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilFile.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilFileTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilGenericParamConstraint.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilGenericParamConstraintTable.h
)
set(ROSETTA_GENERATED_DEFINITIONS_06
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilGenericParam.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilGenericParamTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilImplMap.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilImplMapTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilInstruction.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilInterfaceImpl.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilInterfaceImplTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilManifestResource.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilManifestResourceTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilMemberRef.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilMemberRefTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilMetadata.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilMetadataHeap.h
)
set(ROSETTA_GENERATED_DEFINITIONS_07
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilMetadataRoot.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilMethodDef.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilMethodDefTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilMethodImpl.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilMethodImplTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilMethodSemantics.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilMethodSemanticsTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilMethodSpec.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilMethodSpecTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilModule.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilModuleRef.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilModuleRefTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilModuleTable.h
)
set(ROSETTA_GENERATED_DEFINITIONS_08
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilNestedClass.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilNestedClassTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilNode.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilParam.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilParamTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilProperty.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilPropertyMap.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilPropertyMapTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilPropertyTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilStandAloneSig.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilStandAloneSigTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilTypeDef.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilTypeDefTable.h
)
set(ROSETTA_GENERATED_DEFINITIONS_09
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilTypeRef.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilTypeRefTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilTypeSpec.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilTypeSpecTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilUint32Heap.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilUint8Heap.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCliHeader.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCoffStrtab.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCoffSymbol.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCoffSymbolList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCoffSymbolTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCommonSubExpression.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmConstantExpression.h
)
set(ROSETTA_GENERATED_DEFINITIONS_10
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmControlFlagsExpression.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDirectRegisterExpression.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDOSExtendedHeader.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDOSFileHeader.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfAccessDeclaration.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfArrayType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfBaseType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfCatchBlock.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfClassTemplate.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfClassType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfCommonBlock.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfCommonInclusion.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfCompilationUnit.h
)
set(ROSETTA_GENERATED_DEFINITIONS_11
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfCompilationUnitList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfCondition.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfConstant.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfConstruct.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfConstructList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfConstType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfDwarfProcedure.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfEntryPoint.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfEnumerationType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfEnumerator.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfFileType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfFormalParameter.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfFormatLabel.h
)
set(ROSETTA_GENERATED_DEFINITIONS_12
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfFriend.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfFunctionTemplate.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfImportedDeclaration.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfImportedModule.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfImportedUnit.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfInformation.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfInheritance.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfInlinedSubroutine.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfInterfaceType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfLabel.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfLexicalBlock.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfLine.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfLineList.h
)
set(ROSETTA_GENERATED_DEFINITIONS_13
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfMacro.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfMacroList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfMember.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfModule.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfMutableType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfNamelist.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfNamelistItem.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfNamespace.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfPackedType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfPartialUnit.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfPointerType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfPtrToMemberType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfReferenceType.h
)
set(ROSETTA_GENERATED_DEFINITIONS_14
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfRestrictType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfSetType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfSharedType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfStringType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfStructureType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfSubprogram.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfSubrangeType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfSubroutineType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfTemplateTypeParameter.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfTemplateValueParameter.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfThrownType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfTryBlock.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfTypedef.h
)
set(ROSETTA_GENERATED_DEFINITIONS_15
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfUnionType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfUnknownConstruct.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfUnspecifiedParameters.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfUnspecifiedType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfUpcRelaxedType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfUpcSharedType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfUpcStrictType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfVariable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfVariant.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfVariantPart.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfVolatileType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmDwarfWithStmt.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfDynamicEntry.h
)
set(ROSETTA_GENERATED_DEFINITIONS_16
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfDynamicEntryList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfDynamicSection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfEHFrameEntryCI.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfEHFrameEntryCIList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfEHFrameEntryFD.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfEHFrameEntryFDList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfEHFrameSection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfFileHeader.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfNoteEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfNoteEntryList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfNoteSection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfRelocEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfRelocEntryList.h
)
set(ROSETTA_GENERATED_DEFINITIONS_17
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfRelocSection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSectionTableEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSectionTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSegmentTableEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSegmentTableEntryList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSegmentTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfStringSection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfStrtab.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymbol.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymbolList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymbolSection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymverDefinedAux.h
)
set(ROSETTA_GENERATED_DEFINITIONS_18
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymverDefinedAuxList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymverDefinedEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymverDefinedEntryList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymverDefinedSection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymverEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymverEntryList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymverNeededAux.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymverNeededAuxList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymverNeededEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymverNeededEntryList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymverNeededSection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmElfSymverSection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmExecutableFileFormat.h
)
set(ROSETTA_GENERATED_DEFINITIONS_19
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmExpression.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmExprListExp.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmFloatType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmFloatValueExpression.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmFunction.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmGenericDLL.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmGenericDLLList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmGenericFile.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmGenericFileList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmGenericFormat.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmGenericHeader.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmGenericHeaderList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmGenericSection.h
)
set(ROSETTA_GENERATED_DEFINITIONS_20
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmGenericSectionList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmGenericString.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmGenericStrtab.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmGenericSymbol.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmGenericSymbolList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmIndirectRegisterExpression.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmInstruction.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmInstructionList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmIntegerType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmIntegerValueExpression.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmInterpretation.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmInterpretationList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmAttribute.h
)
set(ROSETTA_GENERATED_DEFINITIONS_21
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmAttributeTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmBootstrapMethod.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmBootstrapMethods.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmClass.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmCodeAttribute.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmConstantPoolEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmConstantPool.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmEnclosingMethod.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmExceptionHandler.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmExceptionTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmField.h
)
set(ROSETTA_GENERATED_DEFINITIONS_22
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmFieldTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmFileHeader.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmIndexedAttr.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmIndexTableAttr.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmInnerClasses.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmInstruction.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmLineNumberTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmLocalVariableEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmLocalVariableTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmLocalVariableTypeEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmLocalVariableTypeTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmMethod.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmMethodParametersEntry.h
)
set(ROSETTA_GENERATED_DEFINITIONS_23
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmMethodParameters.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmMethodTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmModule.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmNode.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmStackMapFrame.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmStackMapTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmStackMapVerificationType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmLEEntryPoint.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmLEEntryTable.h
)
set(ROSETTA_GENERATED_DEFINITIONS_24
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmLEFileHeader.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmLENameTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmLEPageTableEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmLEPageTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmLERelocTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmLESection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmLESectionTableEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmLESectionTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmM68kInstruction.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmMemoryReferenceExpression.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmMipsInstruction.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmNEEntryPoint.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmNEEntryTable.h
)
set(ROSETTA_GENERATED_DEFINITIONS_25
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmNEFileHeader.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmNEModuleTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmNENameTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmNERelocEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmNERelocTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmNESection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmNESectionTableEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmNESectionTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmNEStringTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmNode.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmNullInstruction.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmOperandList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPEExportDirectory.h
)
set(ROSETTA_GENERATED_DEFINITIONS_26
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPEExportEntry.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPEExportEntryList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPEExportSection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPEFileHeader.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPEImportDirectory.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPEImportDirectoryList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPEImportItem.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPEImportItemList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPEImportSection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPERVASizePair.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPERVASizePairList.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPESection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPESectionTableEntry.h
)
set(ROSETTA_GENERATED_DEFINITIONS_27
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPESectionTable.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPEStringSection.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPowerpcInstruction.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmRegisterNames.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmRegisterReferenceExpression.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmRiscOperation.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmScalarType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmStackExpression.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmStatement.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmStaticData.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmStoredString.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmStringStorage.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmSynthesizedDataStructureDeclaration.h
)
set(ROSETTA_GENERATED_DEFINITIONS_28
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmSynthesizedDeclaration.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmSynthesizedFieldDeclaration.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmUnaryExpression.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmUnaryMinus.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmUnaryPlus.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmUnaryRrx.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmUnarySignedExtend.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmUnaryTruncate.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmUnaryUnsignedExtend.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmUserInstruction.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmValueExpression.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmVectorType.h
)
set(ROSETTA_GENERATED_DEFINITIONS_29
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmX86Instruction.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmVoidType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmPointerType.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmAarch64PState.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilExceptionData.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmCilMethodData.h
)
set(ROSETTA_GENERATED_DEFINITIONS_30
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmAnnotationDefault.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmIndexedAttribute.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmRuntimeAnnotation.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmRuntimeAnnotationPair.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmRuntimeAnnotationValue.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmRuntimeParameterAnnotation.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmRuntimeVisibleAnnotations.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmRuntimeVisibilityParamAnnotations.h
  ${ROSETTA_OUTPUT_DIRECTORY}/SgAsmJvmRuntimeInvisibleAnnotations.h
)
set(ROSETTA_GENERATED_DEFINITIONS
  ${ROSETTA_GENERATED_DEFINITIONS_01}
  ${ROSETTA_GENERATED_DEFINITIONS_02}
  ${ROSETTA_GENERATED_DEFINITIONS_03}
  ${ROSETTA_GENERATED_DEFINITIONS_04}
  ${ROSETTA_GENERATED_DEFINITIONS_05}
  ${ROSETTA_GENERATED_DEFINITIONS_06}
  ${ROSETTA_GENERATED_DEFINITIONS_07}
  ${ROSETTA_GENERATED_DEFINITIONS_08}
  ${ROSETTA_GENERATED_DEFINITIONS_09}
  ${ROSETTA_GENERATED_DEFINITIONS_10}
  ${ROSETTA_GENERATED_DEFINITIONS_11}
  ${ROSETTA_GENERATED_DEFINITIONS_12}
  ${ROSETTA_GENERATED_DEFINITIONS_13}
  ${ROSETTA_GENERATED_DEFINITIONS_14}
  ${ROSETTA_GENERATED_DEFINITIONS_15}
  ${ROSETTA_GENERATED_DEFINITIONS_16}
  ${ROSETTA_GENERATED_DEFINITIONS_17}
  ${ROSETTA_GENERATED_DEFINITIONS_18}
  ${ROSETTA_GENERATED_DEFINITIONS_19}
  ${ROSETTA_GENERATED_DEFINITIONS_20}
  ${ROSETTA_GENERATED_DEFINITIONS_21}
  ${ROSETTA_GENERATED_DEFINITIONS_22}
  ${ROSETTA_GENERATED_DEFINITIONS_23}
  ${ROSETTA_GENERATED_DEFINITIONS_24}
  ${ROSETTA_GENERATED_DEFINITIONS_25}
  ${ROSETTA_GENERATED_DEFINITIONS_26}
  ${ROSETTA_GENERATED_DEFINITIONS_27}
  ${ROSETTA_GENERATED_DEFINITIONS_28}
  ${ROSETTA_GENERATED_DEFINITIONS_29}
  ${ROSETTA_GENERATED_DEFINITIONS_30}
)

# ROSETTA-generated headers files. If one of these headers is no longer generated, "make" will repeatedly run the rule
# to try to generate it.
set(ROSETTA_HEADERS
  ${ROSETTA_OUTPUT_DIRECTORY}/AST_FILE_IO.h
  ${ROSETTA_OUTPUT_DIRECTORY}/AstQueryMemoryPool.h
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_Grammar.h
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarDeclarations.h
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarDowncast.h
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarStorageClasses.h
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarSgNodeDefinitions.h
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarSgAsmNodeDefinitions.h
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarSgAsmCilNodeDefinitions.h
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarSgAsmDwarfNodeDefinitions.h
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarSgAsmJvmNodeDefinitions.h
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarVariants.h
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarVisitorSupport.h
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarMemoryPoolSupport.h
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarTreeTraversalAccessEnums.h
  ${ROSETTA_OUTPUT_DIRECTORY}/StorageClasses.h
  ${ROSETTA_GENERATED_DEFINITIONS}
)

# ROSETTA-generated source files.
set(ROSETTA_SRC
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_Grammar.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarCheckingIfDataMembersAreInMemoryPool.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarCopyMemberFunctions.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarGetChildIndex.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarMemoryPoolSupport.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarNewAndDeleteOperators.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarNewConstructors.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarProcessDataMemberReferenceToPointers.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarReturnClassHierarchySubTree.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarReturnDataMemberPointers.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarRTI.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarNodeIdSupport.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarTraverseMemoryPool.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarTreeTraversalSuccessorContainer.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarVariantEnumNames.C
  ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_GrammarVisitorSupport.C
  ${ROSETTA_OUTPUT_DIRECTORY}/StorageClasses.C
  ${ROSETTA_OUTPUT_DIRECTORY}/AST_FILE_IO.C)

# Tell cmake that these files are generated by ROSETTA and will only be available at compile time
set_source_files_properties(${ROSETTA_SRC} ${ROSETTA_HEADERS} PROPERTIES GENERATED 1)

if(MSVC)
  set_source_files_properties(
    ${ROSETTA_OUTPUT_DIRECTORY}/StorageClasses.C
    ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_Grammar.C
    ${ROSETTA_OUTPUT_DIRECTORY}/Cxx_VisitorSupport.C
    PROPERTIES COMPILE_FLAGS /bigobj)
endif()

########################################################################################################################
# Subdirs for the ROSE library
########################################################################################################################

# Build in the subdirectories that make up the ROSE source code
add_subdirectory(util)
add_subdirectory(Sawyer)
add_subdirectory(Rosebud)
add_subdirectory(ROSETTA)
add_subdirectory(roseSupport)
add_subdirectory(3rdPartyLibraries)
add_subdirectory(roseIndependentSupport)

add_definitions(-DROSE_DLL_EXPORTS)

add_subdirectory(roseExtensions)
add_subdirectory(generated)
add_subdirectory(frontend)
add_subdirectory(midend)
add_subdirectory(backend)
add_subdirectory(Rose)
add_subdirectory(AstNodes)

########################################################################################################################
# The ROSE library
########################################################################################################################

# Create an INTERFACE library that OBJECT libraries can link against to get
# include directories from optional dependencies. OBJECT libraries don't inherit
# transitive dependencies from ROSE_DLL, so they need explicit access to headers
# from dependencies that appear in ROSE public headers (like dlib in Matrix.h).
add_library(rose_object_library_dependencies INTERFACE)
if(CAPSTONE_FOUND)
  target_include_directories(rose_object_library_dependencies INTERFACE ${CAPSTONE_INCLUDE_DIR})
endif()
if(DLIB_FOUND)
  target_link_libraries(rose_object_library_dependencies INTERFACE dlib::dlib)
endif()
if(DWARF_FOUND)
  target_include_directories(rose_object_library_dependencies INTERFACE ${DWARF_INCLUDE_DIRS})
endif()
if(GCRYPT_FOUND)
  target_include_directories(rose_object_library_dependencies INTERFACE ${GCRYPT_INCLUDE_DIR})
endif()
if(PQXX_FOUND)
  target_include_directories(rose_object_library_dependencies INTERFACE ${PQXX_INCLUDE_DIR})
endif()
if(SQLITE3_FOUND)
  target_include_directories(rose_object_library_dependencies INTERFACE ${SQLITE3_INCLUDE_DIR})
endif()
if(Z3_FOUND AND TARGET z3::libz3)
  target_link_libraries(rose_object_library_dependencies INTERFACE z3::libz3)
endif()
if(ROSE_ENABLE_FLATBUFFERS)
  target_link_libraries(rose_object_library_dependencies INTERFACE flatbuffers)
endif()


set(rose_LIB_OBJS   
  util_main
  util_commandlineProcessing
  util_sawyer
  util_stringSupport
  util_support
  util_graphs
  unparser
  sage3
  virtualCFG
  astFromString
  roseSupport
  roseDisassemblers
  roseBinaryFormats
  roseAsmUnparser
  astFixup
  midend
  abstractHandle
  abstractLayer
  astDiagnostics
  astProcessing
  astMatching
  midend_util
  midend_binary
  midend_pt
  ExtractFunctionArgumentsNormalization
  SingleStatementToBlockNormalization
  sageInterface
  astTokenStream
  astHiddenTypeAndDeclarationLists
  astVisualization
  includeDirectivesProcessing
  astPostProcessing
  failsafe
  trans_tracking
  astRewriteMechanism
  roseAstNodesBinaryAnalysis
  roseAstNodesExpression
  roseNamespace
  roseCommonAnalysis
  roseAST
  roseBinaryAnalysis
  roseBinaryAnalysisArchitecture
  roseBinaryAnalysisByteCode
  roseBinaryAnalysisCallingConvention
  roseBinaryAnalysisConcolic
  roseBinaryAnalysisConcolicCallback
  roseBinaryAnalysisConcolicI386Linux
  roseBinaryAnalysisConcolicM68kSystem
  roseBinaryAnalysisDebugger
  roseBinaryAnalysisDisassembler
  roseBinaryAnalysisDwarf
  roseBinaryAnalysisInstructionSemantics
  roseBinaryAnalysisInstructionSemanticsBaseSemantics
  roseBinaryAnalysisModelChecker
  roseBinaryAnalysisPartitioner2
  roseBinaryAnalysisSerialization
  roseBinaryAnalysisUnparser
  roseBinaryAnalysisVariables
  roseColor
  roseCommandLine
  roseDiagnostics
  roseFileSystem
  roseSarif
  roseSource
  roseSourceAnalysis
  roseSourceAST
  roseSourceGeneration
  roseStringUtility
  roseGeneratedBinaryAnalysis
  roseGeneratedJovial
  mstl
  libSrcGeneratedRoseSarif)

set(rose_LIB_SRCS
  dummyCppFileForLibrose.C
  ${ROSETTA_SRC})

if(ENABLE_JAVA)
  list(APPEND rose_LIB_OBJS roseJava)
endif()

if(ENABLE_BINARY_ANALYSIS)
  list(APPEND rose_LIB_OBJS roseBinaryAnalysisByteCodeClassHierarchy)
endif()

if(ENABLE_JOVIAL)
  list(APPEND rose_LIB_OBJS roseATerm)
  list(APPEND rose_LIB_OBJS roseTreeBuilder)
  list(APPEND rose_LIB_OBJS roseJovial)
endif()

if(ENABLE_ADA)
  list(APPEND rose_LIB_OBJS experimentalRoseAda)
endif() 

if(ENABLE_LIBADALANG)
  list(APPEND rose_LIB_OBJS experimentalRoseLibadalang)
endif()

if(ENABLE_PHP)
  list(APPEND rose_LIB_OBJS phpFrontend)
endif()

if(NOT ENABLE_INTERNALFRONTENDDEVELOPMENT)
  list(APPEND rose_LIB_OBJS
    midend_pa
    midend_loopproc)
endif()

if(ENABLE_ROSEHPCT)
  list(APPEND rose_LIB_OBJS roseExtensions)
endif()

# Cxx Frontend Conditional: either EDG or Clang or nothing
if(ENABLE_CPP)
  if(NOT ENABLE_CLANG_FRONTEND)  # USE EDG
    message(STATUS "ROSE will be using EDG for the CXX Frontend")
    message(STATUS "  have_EDG_source=${have_EDG_source}, EDG_COMPILE=${EDG_COMPILE}")
    # Only link RoseSourceCxxFrontendEdg if we're compiling EDG from source
    # When using precompiled EDG binaries, the library is linked separately
    if(have_EDG_source AND EDG_COMPILE)
      message(STATUS "  Adding RoseSourceCxxFrontendEdg to rose_LIB_OBJS")
      list(APPEND rose_LIB_OBJS RoseSourceCxxFrontendEdg)
    else()
      message(STATUS "  NOT adding RoseSourceCxxFrontendEdg (using precompiled or dummy EDG)")
    endif()
  else()                         # USE CLANG
    message(STATUS "ROSE will be using Clang for the CXX Frontend")
    list(APPEND rose_LIB_OBJS roseClangFrontend)
    set(BACKEND_CXX_IS_CLANG_COMPILER TRUE)
  endif()
endif()

# Make sure both CxxFrontend options are not enabled
if(EDG_COMPILE AND ENABLE_CLANG_FRONTEND)
  message(FATAL_ERROR "EDG_COMPILE and ENABLE_CLANG_FRONTEND are mutually exclusive."
                      "\nPlease disable one of these options at config time")
endif()

# Apply rose_object_library_dependencies to all OBJECT libraries in rose_LIB_OBJS
# This ensures they all get include directories for optional dependencies (dlib, capstone, etc.)
# that appear in ROSE public headers
foreach(_obj_lib ${rose_LIB_OBJS})
  if(TARGET ${_obj_lib})
    target_link_libraries(${_obj_lib} PRIVATE rose_object_library_dependencies)
  endif()
endforeach()

set(rose_LIB_SRC_FILES dummyCppFileForLibrose.C)

########################################################################################################################
# The ROSE library itself (librose.so, librose.a, librose.dll)
########################################################################################################################
add_library(ROSE_DLL SHARED ${rose_LIB_SRCS})
add_dependencies(ROSE_DLL roseUtil)

set_target_properties(ROSE_DLL PROPERTIES
  OUTPUT_NAME "rose"
  VERSION "${PROJECT_VERSION}"   # e.g., 0.11.145.348
  SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"  # e.g., 0.11 for backward compatibility between 0.11.a.b and 0.11.c.d
)

# Create namespaced alias for modern CMake usage
# This allows projects using ROSE to reference it as Rose::rose instead of ROSE_DLL.
# Benefits:
#   1. Follows modern CMake best practices (Package::component naming convention)
#   2. Prevents naming conflicts with other libraries
#   3. Makes code more self-documenting (Rose::rose clearly indicates ROSE library)
#   4. Enables CMake to detect typos at configuration time rather than link time
#   5. Consistent with how other modern libraries work (e.g., Boost::filesystem, Qt5::Core)
# The alias only works in the build tree; exported targets use the same namespace via EXPORT NAMESPACE.
add_library(Rose::rose ALIAS ROSE_DLL)
message(STATUS "Created alias target Rose::rose")

# Propagate include directories to consumers
# ============================================
# This is part of CMake modernization (Step 3 in docs/CMakeModernizationPlan.md).
#
# PURPOSE:
# When external projects link against Rose::rose, they should automatically get the correct
# include paths without needing to manually call include_directories(). This is the modern
# CMake way of handling includes - they are properties of the target that propagate automatically.
#
# WHAT THIS REPLACES:
# Old approach: External projects had to manually find ROSE headers with find_path() and then
# call include_directories(). This was error-prone and required users to know ROSE's internal
# directory structure.
#
# New approach: Just link with target_link_libraries(mytarget PRIVATE Rose::rose) and includes
# are automatically added to the compilation.
#
# GENERATOR EXPRESSIONS EXPLAINED:
# The $<BUILD_INTERFACE:...> and $<INSTALL_INTERFACE:...> syntax are "generator expressions"
# that let us specify different paths for different scenarios:
#
# - BUILD_INTERFACE: Used when building against ROSE in its build directory (e.g., during
#   ROSE development or when a project adds ROSE as a subdirectory with add_subdirectory()).
#   These paths point to the source tree and build tree locations.
#
# - INSTALL_INTERFACE: Used when building against an installed ROSE (the normal case for
#   external projects). These paths are relative to the install prefix and point to where
#   headers are installed (typically /usr/local/include/rose or similar).
#
# PUBLIC vs PRIVATE vs INTERFACE:
# We use PUBLIC because:
# - ROSE's public headers (rose.h, SageIII headers, etc.) need to be available to any code
#   that includes ROSE headers
# - If we used PRIVATE, only ROSE's own compilation would get these paths, not external projects
# - If we used INTERFACE, ROSE itself wouldn't get these paths, only consumers would
# - PUBLIC = both ROSE and its consumers get these paths
#
# WHICH DIRECTORIES ARE INCLUDED:
# - ${ROSE_TOP_SRC_DIR}/src: Main source directory (for #include <rose.h>, etc.)
# - ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII: SageIII AST headers (Sg*.h files)
# - ${ROSE_TOP_SRC_DIR}/src/Rose: Modern Rose namespace headers
# - ${ROSE_TOP_SRC_DIR}/src/util: Utility headers (ROSE_DEPRECATED.h, etc.)
# - ${ROSE_TOP_BINARY_DIR}: Build directory (for generated headers like rosePublicConfig.h)
# - ${ROSE_TOP_BINARY_DIR}/src/frontend/SageIII: Generated SageIII headers
# - ${ROSE_TOP_BINARY_DIR}/src/util: Generated utility headers (rose_paths.h)
# - ${INCLUDE_INSTALL_DIR}: Install location for headers (typically include/rose)
# - ${INCLUDE_INSTALL_DIR}/Rose: Install location for Rose namespace headers
#
target_include_directories(ROSE_DLL PUBLIC
  # Build tree includes - these are used when building against ROSE in its build directory
  # Using ROSE_INCLUDES variable which contains the comprehensive list of all ROSE include directories
  "$<BUILD_INTERFACE:${ROSE_INCLUDES}>"
  # Install tree includes - these are used when building against an installed ROSE
  $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
  $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/Rose>
)

# Set compile features - ROSE requires C++14
# ===========================================
# This tells CMake (and downstream projects) that ROSE requires C++14 as a minimum.
#
# EFFECTS:
# - When compiling ROSE itself, CMake will add -std=c++14 (or equivalent for other compilers)
# - When external projects link against Rose::rose, their code will also be compiled with
#   C++14 or later (because PUBLIC propagates this requirement)
# - If a project tries to use C++11 with ROSE, CMake will upgrade it to C++14 automatically
#
# WHY PUBLIC:
# ROSE's public headers use C++14 features, so any code that includes ROSE headers must
# also be compiled with C++14 or later. PUBLIC ensures this requirement propagates to
# consumers of the library.
#
target_compile_features(ROSE_DLL PUBLIC cxx_std_14)

# Link dependencies with proper PUBLIC/PRIVATE classification
# ============================================================
# This is part of CMake modernization (Step 4 in docs/CMakeModernizationPlan.md).
#
# PURPOSE:
# Properly classify dependencies as PUBLIC or PRIVATE to control transitive dependencies.
# - PUBLIC: Types/symbols from this library appear in ROSE's public headers
# - PRIVATE: Only used internally by ROSE implementation
#
# WHY THIS MATTERS:
# - Reduces unnecessary dependencies for downstream projects
# - Improves link times by not propagating unnecessary libraries
# - Allows ROSE's internal dependencies to change without breaking downstream projects
# - Makes it clear which dependencies are truly required vs. optional
#
# CLASSIFICATION RATIONALE:
#
# PUBLIC dependencies:
# - Boost: Extensively used in ROSE public headers, especially in Rose::BinaryAnalysis
#   (boost::filesystem, boost::graph, boost::serialization, boost::tribool, etc.)
#   Many ROSE APIs expose Boost types directly.
# - rose_LIB_OBJS: ROSE's internal object libraries that contain public APIs
# - link_with_libraries: Contains system libraries (pthreads, dl, m, c, rt) and
#   platform-specific libraries that are part of ROSE's public interface
#
# PRIVATE dependencies (added below):
# - Z3: SMT solver used internally for symbolic execution
# - DWARF/ELF: Binary analysis internal implementation
# - Capstone: Disassembly backend
# - Dlib: Internal matrix/graph algorithms
# - GCrypt: Cryptographic hashing internals
# - Zlib: Compression (though Boost may expose this transitively)
# - POET: Program transformation DSL
# - Language frontends: Python, Java, Fortran, Ada (internal)
#
# Filter out Boost libraries from link_with_libraries to avoid duplication
# We use modern Boost::component targets instead of ${Boost_LIBRARIES}
set(_rose_system_libraries "")
foreach(_lib ${link_with_libraries})
  # Skip Boost libraries (they're added as modern targets below)
  if(NOT "${_lib}" MATCHES "^Boost::" AND NOT "${_lib}" MATCHES "boost_")
    list(APPEND _rose_system_libraries "${_lib}")
  endif()
endforeach()

target_link_libraries(ROSE_DLL
  PUBLIC
    # Boost is in ROSE's public API - many headers expose Boost types
    # Using modern Boost::component targets for proper CMake export
    Boost::chrono
    Boost::date_time
    Boost::filesystem
    Boost::iostreams
    Boost::program_options
    Boost::random
    Boost::regex
    Boost::system
    Boost::wave
    Boost::thread
    Boost::serialization
  PUBLIC
    # Threads is PUBLIC because:
    # 1. Boost::thread requires pthread support
    # 2. ROSE's public API uses threading primitives
    # 3. External projects need -pthread flag for proper linking
    # Note: Threads::Threads is found in top-level CMakeLists.txt when Boost::thread is found
    Threads::Threads
  PUBLIC
    # Internal ROSE object libraries - these object files are implementation details
    # that get compiled into librose.so, but their PUBLIC include directories need to
    # be propagated to ROSE consumers (e.g., tools like CodeThorn that include headers
    # from these components)
    ${rose_LIB_OBJS}
  PRIVATE
    # System and platform libraries (rt, dl, m, c, etc.)
    # Note: Threads moved to PUBLIC above
    ${_rose_system_libraries}
)

# Explicitly add -pthread flag for both compilation and linking
# This is necessary even when CMAKE_THREAD_LIBS_INIT is empty (threading in libc)
# because -pthread affects preprocessor defines, atomics, and mutex behavior.
# Must be PUBLIC so external projects also get -pthread.
if(Threads_FOUND AND NOT WIN32)
  target_compile_options(ROSE_DLL PUBLIC -pthread)
  target_link_options(ROSE_DLL PUBLIC -pthread)
endif()

set_target_properties(ROSE_DLL PROPERTIES
  DEFINE_SYMBOL ROSE_DLL_EXPORTS
  EXPORT_NAME rose)

# Install the ROSE library with export information
# This is part of CMake modernization (Step 5 in docs/CMakeModernizationPlan.md).
# The EXPORT clause creates a RoseTargets export set that will be used to generate
# RoseTargets.cmake, allowing external projects to import Rose::rose as a target.
#
# Note: All OBJECT libraries linked as PUBLIC dependencies must be in the export set.
# This includes rose_LIB_OBJS and any conditionally-added targets like RoseSourceCxxFrontendEdg.
# RoseSourceCxxFrontendEdg is already in rose_LIB_OBJS when EDG is compiled from source,
# so we don't need to add it again here.
# Also include rose_object_library_dependencies which OBJECT libraries link against.
set(_rose_export_targets ROSE_DLL rose_object_library_dependencies ${rose_LIB_OBJS})

install(TARGETS ${_rose_export_targets}
  EXPORT RoseTargets
  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
  RUNTIME DESTINATION ${BIN_INSTALL_DIR}
  INCLUDES DESTINATION ${INCLUDE_INSTALL_DIR}
)

# Install the export set
# This generates RoseTargets.cmake which contains the imported target definitions.
# The NAMESPACE Rose:: ensures that the imported target is named Rose::rose,
# matching the alias we created for the build tree.
install(EXPORT RoseTargets
  FILE RoseTargets.cmake
  NAMESPACE Rose::
  DESTINATION ${LIB_INSTALL_DIR}/cmake/Rose
)
install(
  FILES
    featureTests.h RoseFirst.h rose.h roseInternal.h rose_msvc.h msvc_stdint.h
    ${CMAKE_BINARY_DIR}/rosePublicConfig.h
  DESTINATION ${INCLUDE_INSTALL_DIR})

# Install the parse table for the Jovial front-end
if(ENABLE_JOVIAL)
  install(FILES ${CMAKE_SOURCE_DIR}/src/3rdPartyLibraries/jovial-parser/share/rose/Jovial.tbl DESTINATION share/rose)
endif()

########################################################################################################################
# Dependencies of the ROSE library. librose depends on additional libraries.
########################################################################################################################
# rose_library_dir_list contains libraries required at runtime, these may be added to RPATH
set(rose_library_dir_list "")
set(lib_dir "")
macro(add_library_dependency lib)
  get_filename_component(lib_dir ${lib} DIRECTORY)
  list(APPEND rose_library_dir_list ${lib_dir})
endmacro()

# Link optional dependencies
# ==========================
# Dependencies that are used internally by ROSE and are not exposed in public headers should be marked PRIVATE, otherwise
# mark them as PUBLIC.

# Z3 SMT solver library (optional dependency)
# Modern CMake: use the imported target z3::libz3 provided by Z3Config.cmake
# Z3_FOUND is set by find_package(Z3 CONFIG) in FindZ3.cmake
if(Z3_FOUND AND TARGET z3::libz3)
  target_link_libraries(ROSE_DLL PUBLIC z3::libz3)
  # Note: No need for add_library_dependency with imported targets
endif()

# Dlib is a C++ support library. ROSE uses matrices and some graph algorithms currently.
# NOTE: Marked PUBLIC because ROSE public headers include <dlib/matrix.h>
# (Rose/BinaryAnalysis/Matrix.h and others)
if(DLIB_FOUND)
  target_link_libraries(ROSE_DLL PUBLIC dlib::dlib)
endif()

# Capstone is used to disassemble ARM.
# NOTE: Marked PUBLIC because ROSE public headers include <capstone/*.h>
# (InstructionEnumsAarch32.h, InstructionEnumsAarch64.h, Disassembler/Aarch32.h, Disassembler/Aarch64.h)
if(CAPSTONE_FOUND)
  target_link_libraries(ROSE_DLL PUBLIC ${CAPSTONE_LIBRARY})
  add_library_dependency(${CAPSTONE_LIBRARY})
endif()

# libdwarf and libelf are used to parse DWARF (debugging) information from ELF files. The libelf library is a dependency of
# the libdwarf library, but is not used directly by ROSE (ROSE has its own ELF parser).
if(DWARF_FOUND)
  target_link_libraries(ROSE_DLL PRIVATE ${DWARF_LIBRARY} ${ELF_LIBRARY})
  add_library_dependency(${DWARF_LIBRARY})
  add_library_dependency(${ELF_LIBRARY})
endif()

# The GNU GCrypt library is used for computing a wide variety of hashes. It needs the GPG-Error library.
# NOTE: Marked PUBLIC because ROSE public headers conditionally include <gcrypt.h>
# (InstructionSemantics/PartialSymbolicSemantics.h when ROSE_HAVE_LIBGCRYPT is defined)
if(GCRYPT_FOUND AND GPGERROR_FOUND)
  target_link_libraries(ROSE_DLL PUBLIC ${GCRYPT_LIBRARY})
  add_library_dependency(${GCRYPT_LIBRARY})
  target_link_libraries(ROSE_DLL PUBLIC ${GPGERROR_LIBRARY})
  add_library_dependency(${GPGERROR_LIBRARY})
endif()

# Zlib is a compression library and is needed by Boost. Make sure you compiled Boost with this same zlib.
if(ZLIB_FOUND)
  target_link_libraries(ROSE_DLL PRIVATE ${ZLIB_LIBRARY})
  add_library_dependency(${ZLIB_LIBRARY})
endif()

# If POET is enabled in ROSE, we need to link in some additional dependencies.
if(ENABLE_POET)
  set(LINK_FILES poet ${CMAKE_DL_LIBS})
  add_dependencies(ROSE_DLL poet)
else()
  set(LINK_FILES ${CMAKE_DL_LIBS})
endif()

# ???
if(NOT BINARY_EDG)
  add_definitions(-DUSE_FAKE_EDG)
endif()


# Link language frontend dependencies as PRIVATE
# ===============================================
# Language frontends are implementation details and not exposed in ROSE's public API.

# Depending on which language analysis (frontend) features are enabled in ROSE, we may need some additional libraries.
if(ENABLE_FORTRAN)
  list(APPEND LINK_FILES roseFortran)
endif()

if(ENABLE_JAVA)
  list(APPEND LINK_FILES ${JAVA_JVM_LIBRARY})
endif()

if(ENABLE_PYTHON)
  list(APPEND LINK_FILES unparsePython rosePythonFrontend)
endif()

if(ENABLE_ROSEHPCT)
  list(APPEND LINK_FILES ${LIBXML2_LIBRARIES})
endif()

if(ENABLE_ADA)
  target_link_libraries(ROSE_DLL PRIVATE ${ASIS_ADAPTER_LIB})
  list(APPEND LINK_FILES experimentalRoseAda)
endif()

if(ENABLE_LIBADALANG)
  list(APPEND rose_library_dir_list ${ROSE_EXPERIMENTAL_LIBADALANG_ROSE_CONNECTION_GNAT_HOME}/lib)
  list(APPEND rose_library_dir_list /opt/rosedev/GNAT/2021/lib/gcc/x86_64-pc-linux-gnu/10.3.1/adalib)
  #HACK
endif()


set(ROSE_TARGET_LINK ${LINK_FILES})
if(WIN32)
  target_link_libraries(ROSE_DLL PRIVATE ${ROSE_TARGET_LINK} shlwapi.lib Ws2_32.lib)
else ()
  target_link_libraries(ROSE_DLL PRIVATE ${ROSE_TARGET_LINK})
endif()

# If the C preprocessor is enabled, ROSE has some additional depenendencies
if(ENABLE_CPP)
  add_dependencies(ROSE_DLL OMPPARSER ROSE_PREPROCESSOR)
endif()

# SQLite3 has an executable component and a development (library and headers) component that can be installed
# independently of each other. Therfore SQLITE3_FOUND_EXE says whether the executable component was found, and
# SQLITE3_FOUND_LIB says whether the development component was found; SQLITE3_FOUND is set if either component is found.
if(SQLITE3_FOUND_LIB)
  # CORY: Maybe you can help. When sqlite is enabled, I get errors from cmake (when run in a clean build directory) that
  #       say "Cannot find source file /home/matzke/junk/_build-cmake/src/util/CMakeFiles/util_main.dir/rose_paths.C.o;
  #       Tried extensions .c .C .c++ .cc .cpp .cxx .m .M .mm .h .hh .h++ .hm .hpp .hxx .in .txx". I'm not sure why
  #       cmake things "rose_paths.C.o" should be a source file. The $ROSE/src/util/CMakeLists.txt has the CMake
  #       commands for creating rose_paths.C and rose_paths.h, and is the only CMakeLists.txt file that mentions
  #       "rose_paths". The error goes away if I comment out the following line (but then of course there's linking
  #       problems later).
  target_link_libraries(ROSE_DLL PRIVATE RoseSQLite3xDatabase)
endif()

########################################################################################################################
# testSharedRoseLib -- a basic test to make sure the ROSE library works
########################################################################################################################
remove_definitions(-DROSE_DLL_EXPORTS)
add_executable(testSharedRoseLib testRoseLib.C)
target_link_libraries(testSharedRoseLib ROSE_DLL ${link_with_libraries})

########################################################################################################################
# The rose-config tool. DEPRECATED 2025-10-10: Use pkg-config or RoseConfig.cfg
########################################################################################################################
add_definitions(-DLIBDIR="${CMAKE_INSTALL_PREFIX}/${ROSE_LIB_DIR_NAME}") #LIB_INSTALL_DIR ?
add_executable(rose-config rose-config.C)
target_link_libraries(rose-config ROSE_DLL ${link_with_libraries})
install(TARGETS rose-config DESTINATION bin)

########################################################################################################################
# The rose-compiler tool
########################################################################################################################
add_executable(rose-compiler rose-compiler.C)
target_link_libraries(rose-compiler ROSE_DLL ${link_with_libraries})
install(TARGETS rose-compiler DESTINATION bin)

########################################################################################################################
# check-core target. Tests a simple program for each language that is enabled
########################################################################################################################
add_custom_target(
  check-core
  COMMENT "Running check-core")
add_dependencies(check-core rose-compiler)

#Check that Boost is linked
add_custom_command(
  TARGET check-core
  POST_BUILD
  COMMENT "  TEST     boost shared linkage [rose-compiler]"
  COMMAND ${ROSE_TOP_SRC_DIR}/scripts/check_boost_linkage.sh ${PROJECT_BINARY_DIR}/bin/rose-compiler >&2
  DEPENDS rose-compiler)

#Check that the other links are correct
add_custom_command(
  TARGET check-core
  POST_BUILD
  COMMENT "  TEST     whether a trivial ROSE program runs [rose-compiler]"
  COMMAND /bin/sh -c "if ! ${PROJECT_BINARY_DIR}/bin/rose-compiler --version ; then echo \"error: You cannot run a program linked with ROSE. Please be sure that your\" >&2; echo \"error: LD_LIBRARY_PATH (Linux) or DYLD_LIBRARY_PATH (Mac OSX) contains the\" >&2; echo \"error: correct libraries (JVM, Boost, librose.so, etc.)\" >&2; exit 1; fi;"
  VERBATIM
  DEPENDS rose-compiler)

#Check that rose-config works
add_custom_command(
  TARGET check-core
  POST_BUILD
  COMMENT "  TEST     ROSE configuration tool [rose-config]"
  COMMAND rose-config --version
  VERBATIM
  DEPENDS rose-config)

if(ENABLE_C)
  add_custom_target(check-hello-cxx)
  add_dependencies(check-hello-cxx rose-compiler)

  #Check that we can compile & run a simple C program
  add_custom_command(
    TARGET check-core
    POST_BUILD
    COMMAND echo "\#include <stdio.h>"               >  hello-world.c
    COMMAND echo "int main() {"                      >> hello-world.c
    COMMAND echo "   puts(\"Hello World from C!\");" >> hello-world.c
    COMMAND echo "   return 0;"                      >> hello-world.c
    COMMAND echo "}"                                 >> hello-world.c
    COMMAND rose-compiler -rose:ast:write hello-world-c.ast hello-world.c -o hello-world-c
    COMMAND ./hello-world-c
    COMMAND rose-compiler -rose:ast:read hello-world-c.ast
    COMMAND rm -f hello-world.c hello-world-c hello-world-c.ast hello-world.o rose_hello-world.c
      rose_odr_class.log rose_odr_function.log rose_odr_variable.log
    VERBATIM
    DEPENDS rose-compiler
    COMMENT "  TEST     ROSE C compiler")

  #Check that we can compile & run a simple C++ program
  add_custom_command(
    TARGET check-core check-hello-cxx
    POST_BUILD
    COMMAND echo "\#include <iostream>"                                     >  hello-world.cpp
    COMMAND echo "int main() {"                                             >> hello-world.cpp
    COMMAND echo "    std::cout << \"Hello World from C++!\" << std::endl;" >> hello-world.cpp
    COMMAND echo "    return 0;"                                            >> hello-world.cpp
    COMMAND echo "}"                                                        >> hello-world.cpp
    COMMAND rose-compiler -rose:ast:write hello-world-cxx.ast hello-world.cpp -o hello-world-cpp
    COMMAND ./hello-world-cpp
    COMMAND rose-compiler -rose:ast:read hello-world-cxx.ast
    COMMAND rm -f hello-world.cpp hello-world-cpp hello-world-cxx.ast hello-world.o hello-world.ti
      rose_hello-world.cpp rose_odr_class.log rose_odr_function.log rose_odr_variable.log
    VERBATIM
    DEPENDS rose-compiler
    COMMENT "  TEST     ROSE C++ compiler")
endif()

if(ENABLE_FORTRAN)
  #Check that we can compile and run a simple fortran program
  add_custom_command(
    TARGET check-core
    POST_BUILD
    COMMAND echo "program hello"                            >  hello-world.f90
    COMMAND echo "  print *, \"Hello World from Fortran!\"" >> hello-world.f90
    COMMAND echo "end program hello"                        >> hello-world.f90
    COMMAND rose-compiler -rose:ast:write hello-world-f90.ast hello-world.f90 -o hello-world-f90
    COMMAND ./hello-world-f90
#    COMMAND rose-compiler -rose:ast:read hello-world-f90.ast
    COMMAND rm -f hello-world.f90 hello-world-f90 hello-world-f90.ast hello-world.o rose_hello-world.f90
    VERBATIM
    DEPENDS rose-compiler
    COMMENT "  TEST     ROSE Fortran compiler")
endif()

if(ENABLE_ADA)
  #Check that we can compile and run a simple ada program
  add_custom_command(
    TARGET check-core
    POST_BUILD
    COMMAND echo "with Ada.Text_IO;"                        >  hello-world.adb
    COMMAND echo "procedure Hello_World is"                 >> hello-world.adb
    COMMAND echo "begin"                                    >> hello-world.adb
    COMMAND echo "  Ada.Text_IO.Put_Line(\"Hello World\");" >> hello-world.adb
    COMMAND echo "end Hello_World;"                         >> hello-world.adb
    COMMAND rose-compiler hello-world.adb
# just test frontend/backend in ROSE
#~    COMMAND ./hello-world-f90
#    COMMAND rose-compiler -rose:ast:read hello-world-f90.ast
    COMMAND rm -f hello-world.adb
    VERBATIM
    DEPENDS rose-compiler
    COMMENT "  TEST     ROSE Ada compiler")
endif()

if(ENABLE_BINARY_ANALYSIS)
  #Check that we can perform a simple binary analysis
  add_custom_command(
    TARGET check-core
    POST_BUILD
    COMMAND  /bin/sh -c "if [ ! -f ${PROJECT_BINARY_DIR}/bin/bat-ana ] ; then echo \"error: bat-ana is not present. Check that you ran 'make tools'.\" >&2; exit 1; fi;"
    COMMAND echo "\#include <stdio.h>"               >  hello-world.c
    COMMAND echo "int main() {"                      >> hello-world.c
    COMMAND echo "   puts(\"Hello World from C!\");" >> hello-world.c
    COMMAND echo "   return 0;"                      >> hello-world.c
    COMMAND echo "}"                                 >> hello-world.c
    COMMAND ${CMAKE_C_COMPILER} -c hello-world.c -o hello-world-c
    COMMAND ${PROJECT_BINARY_DIR}/bin/bat-ana -o hello-world-ba hello-world-c
    COMMAND rm -f hello-world.c hello-world-c hello-world-ba
    VERBATIM
    DEPENDS tools
    COMMENT "  TEST     ROSE Binary analysis")
endif()

########################################################################################################################
# rose-config.cfg -- text file that contains various settings for user-level makefiles.
# DEPRECATED 2025-10-10. Use `pkg-config` or "RoseConfig.cmake" instead.
########################################################################################################################

########## Build cppflags for ROSE tools ###############################################################################
set(rose_config_installed_cppflags "-I${CMAKE_INSTALL_PREFIX}/include/rose ")

list(APPEND rose_library_dir_list "${Boost_LIBRARY_DIRS}")
if(NOT ${ATERM_LIBRARY} STREQUAL "ATERM_LIBRARY-NOTFOUND")
  if(NOT ${ATERM_LIBRARY} STREQUAL "")
    add_library_dependency(${ATERM_LIBRARY})
    target_link_libraries(ROSE_DLL PRIVATE ${ATERM_LIBRARY})
    set(rose_config_installed_cppflags "${rose_config_installed_cppflags} -I${ATERM_LIBRARY_DIR}/../include")
  endif()
endif()
# Add -pthread if Threads_FOUND or Boost::thread is found to avoid rose-tool segfaults 
if(Threads_FOUND OR "${Boost_LIBRARIES}" MATCHES "Boost::thread")
  set(rose_config_installed_cppflags "${rose_config_installed_cppflags} -pthread")
endif()

# Include software dependencies
if(GCRYPT_FOUND)
  set(rose_config_installed_cppflags "${rose_config_installed_cppflags} -I${GCRYPT_HEADER_PATH}")
endif()
if(GPGERROR_FOUND)
  set(rose_config_installed_cppflags "${rose_config_installed_cppflags} -I${GPGERROR_HEADER_PATH}")
endif()
if(Z3_FOUND_LIB)
  set(rose_config_installed_cppflags "${rose_config_installed_cppflags} -I${Z3_HEADER_PATH}")
endif()

# Fix double slashes in rose-config
if("${Boost_INCLUDE_DIRS}" MATCHES "^/")
  # this var starts with a slash so we dont append another one after the -I 
  set(rose_config_installed_cppflags "${rose_config_installed_cppflags} -I${Boost_INCLUDE_DIRS}")
else()
  set(rose_config_installed_cppflags "${rose_config_installed_cppflags} -I/${Boost_INCLUDE_DIRS}")
endif() 
########################################################################################################################

########### Build ldflags for ROSE tools ###############################################################################
## ATTENTION:  This order matters, please be very careful if modifying  ## 
set(rose_config_installed_ldflags "${rose_config_installed_ldflags} -L${CMAKE_INSTALL_PREFIX}/${ROSE_LIB_DIR_NAME}") #-L
set(rose_library_list "rose") # -l 
# ONLY REQUIRED DEPENDENCIES 
set(rose_library_list "${rose_library_list};${Boost_LIBRARIES}")
foreach(X IN LISTS rose_library_list)
  # Dereference ${X} with "" to fix Windows CI/CD error 
  get_filename_component(Y "${X}" NAME_WE)  
  # If Y is empty, use X directly
  if(Y STREQUAL "") 
    set(Y "${X}")
  endif() 
  # Handle non-path library names appropriately
  string(REGEX REPLACE "^-l" "" Y "${Y}")  # Remove '-l' prefix if present
  string(REGEX REPLACE "^lib" "" Y "${Y}") # Remove 'lib' prefix if present
  string(REGEX REPLACE "^Boost::" "boost_" Y "${Y}") # Convert boost modules into ld-recognizeable libraries
  # Avoid treating empty strings as library names
  if(NOT Y STREQUAL "")
    set(rose_config_installed_ldflags "${rose_config_installed_ldflags} -l${Y}")
  endif()
endforeach()
# Add -ldl before -lm if HAVE_LIBDL
if(HAVE_LIBDL)
  string(REPLACE " -lm " " -ldl -lm " rose_config_installed_ldflags "${rose_config_installed_ldflags}")
endif()
# Add -pthread to avoid rose-tool segfaults 
if(Threads_FOUND OR "${Boost_LIBRARIES}" MATCHES "Boost::thread")
  string(REPLACE " -lrose " " -lrose -pthread " rose_config_installed_ldflags "${rose_config_installed_ldflags}")
endif()
# Add Boost dir before boost libs to maximize portability by making sure boost is only looked for in one location 
string(REPLACE " -lboost_chrono " " -L${Boost_LIBRARY_DIRS} -lboost_chrono " rose_config_installed_ldflags "${rose_config_installed_ldflags}")
########################################################################################################################

# Function for rose-config to ensure rose tools use correct compiler 
function(resolve_compiler_names)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(cc_name "gcc" PARENT_SCOPE)
        set(cxx_name "g++" PARENT_SCOPE)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        set(cc_name "clang" PARENT_SCOPE)
        set(cxx_name "clang++" PARENT_SCOPE)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
        set(cc_name "icc" PARENT_SCOPE)
        set(cxx_name "icpc" PARENT_SCOPE)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "XL")
        set(cc_name "xlc" PARENT_SCOPE)
        set(cxx_name "xlC" PARENT_SCOPE)
    endif()
endfunction()

# Usage example
resolve_compiler_names()

# Use cc_name if set, otherwise default to BACKEND_C_COMPILER_NAME_WITHOUT_PATH
if(cc_name)
    set(ACTUAL_CC ${cc_name})
else()
    set(ACTUAL_CC ${BACKEND_C_COMPILER_NAME_WITHOUT_PATH})
endif()

# Use cxx_name if set, otherwise default to BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH
if(cxx_name)
    set(ACTUAL_CXX ${cxx_name})
else()
    set(ACTUAL_CXX ${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH})
endif()

# Remove duplicates and any system directories
list(REMOVE_DUPLICATES rose_library_dir_list)
list(FILTER rose_library_dir_list EXCLUDE REGEX "^/lib.*")
list(FILTER rose_library_dir_list EXCLUDE REGEX "^/usr/lib.*")
list(FILTER rose_library_dir_list EXCLUDE REGEX "^/usr/local/lib.*")
list(FILTER rose_library_dir_list EXCLUDE REGEX "^/var/lib.*")

set(ROSE_LIBDIRS "")
set(ROSE_LINK_RPATHS "")
set(ROSE_RPATHS "")

foreach(X IN LISTS rose_library_dir_list)
  set(ROSE_LIBDIRS "${ROSE_LIBDIRS} ${X}")
  set(ROSE_LINK_RPATHS "${ROSE_LINK_RPATHS} -Wl,-rpath ${X}")
  set(ROSE_RPATHS "${ROSE_RPATHS} -R ${X}")
endforeach()

if(CMAKE_BUILD_TYPE STREQUAL "Debug") 
  message(STATUS "rose_library_dir_list: ${rose_library_dir_list}")
  message(STATUS "ROSE_LIBDIRS: ${ROSE_LIBDIRS}")
  message(STATUS "ROSE_LINK_RPATHS: ${ROSE_LINK_RPATHS}")
  message(STATUS "ROSE_RPATHS: ${ROSE_RPATHS}")
endif() 

# CMake should handle this for us by default with CMAKE_CXX_STANDARD 
# set(ROSE_CXXFLAGS "${CMAKE_CXX_FLAGS} -std=c++${CMAKE_CXX_STANDARD}" )
# uncommenting the line above MAY break some tools

set(ROSE_CONFIG_FILE "${ROSE_TOP_BINARY_DIR}/src/rose-config.cfg") 
file(REMOVE ${ROSE_CONFIG_FILE})
file(WRITE  ${ROSE_CONFIG_FILE} "# DEPRECATED 2025-10-10. Use pkg-config or RoseConfig.cmake instead\n\n\n")
file(APPEND ${ROSE_CONFIG_FILE} "# Things that the 'rose-config' tool can report\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_CC           = ${ACTUAL_CC}\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_CCPATH       = ${CMAKE_C_COMPILER}\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_CXX          = ${ACTUAL_CXX}\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_CXX_PATH     = ${CMAKE_CXX_COMPILER}\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_CPPFLAGS     = ${rose_config_installed_cppflags}\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_CFLAGS       = ${CMAKE_C_FLAGS}\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_CXXFLAGS     = ${CMAKE_CXX_FLAGS}\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_LDFLAGS      = ${rose_config_installed_ldflags}\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_LIBDIRS      = ${ROSE_LIBDIRS}\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_PREFIX       = ${CMAKE_INSTALL_PREFIX}\n")
file(APPEND ${ROSE_CONFIG_FILE} "# Additional items useful to include in Makefiles\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_RPATHS       = ${ROSE_RPATHS}\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_LINK_RPATHS  = ${ROSE_LINK_RPATHS}\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_VERSION      = ${ROSE_PACKAGE_VERSION}\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_BACKEND_C_COMPILER      = ${BACKEND_C_COMPILER}\n")
file(APPEND ${ROSE_CONFIG_FILE} "ROSE_BACKEND_CXX_COMPILER    = ${BACKEND_CXX_COMPILER}\n")
install(FILES ${ROSE_CONFIG_FILE} DESTINATION ${ROSE_LIB_DIR_NAME})

########### Code for new testing interface #############################################################################
# Initialize counters for macro below (only for debugging purposes)
set(TRUE_COUNT 0)   # New variables written to rose config.cfg
set(FALSE_COUNT 0)  # New variables processed but not written to rose config.cfg
set(TOTAL_COUNT 0)  # Total new variables that could be written to rose config.cfg

# Macro to append boolean configuration values to rose_config.cfg
macro(append_bool_config VAR_NAME VAR_VALUE)
  math(EXPR TOTAL_COUNT "${TOTAL_COUNT} + 1")     # Increment total count (for debugging) 
  if(${VAR_VALUE})
    file(APPEND ${ROSE_CONFIG_FILE} "${VAR_NAME} = true\n")
    math(EXPR TRUE_COUNT "${TRUE_COUNT} + 1")   # Increment true count  (for debugging)
  else()
    file(APPEND ${ROSE_CONFIG_FILE} "${VAR_NAME} = false\n")
    math(EXPR FALSE_COUNT "${FALSE_COUNT} + 1") # Increment false count (for debugging)
  endif()
endmacro()

# Set Automake variableis which are used by standalone tests   
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  set(USING_GNU_COMPILER 1)    
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "4.4")
    set(ROSE_USING_GCC_VERSION_LATER_4_4 1)
    set(ROSE_USING_GFORTRAN_VERSION_LATER_4_4 1)
    set(ROSE_USING_GFORTRAN_VERSION_LATER_4_5 1)
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "4.9")
    set(ROSE_USING_GCC_VERSION_LATER_4_8 1)
    set(ROSE_USING_GCC_VERSION_LATER_4_9 1)
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "5.1")
    set(ROSE_USING_GCC_VERSION_LATER_5_1 1)
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "6.0")
    set(ROSE_USING_GCC_VERSION_LATER_6_0 1)
    set(ROSE_USING_GCC_VERSION_LATER_6_1 1) 
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "10.1")
    set(ROSE_USING_GCC_VERSION_LATER_10_1 1)
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "10.5")
    set(ROSE_USING_GCC_VERSION_LATER_10_5 1)
  endif()
endif() 

if(EDG_VERSION STREQUAL "5.0")
  set(ROSE_USE_EDG_VERSION_5_0 1) 
endif() 

# Don't have appropriate conditional for this 
set(ROSE_USE_VXWORKS 1) 

# Check if the current compiler default C++ standard is 11 or greater
if(CMAKE_CXX_STANDARD AND CMAKE_CXX_STANDARD GREATER_EQUAL 11)
  set(ROSE_FRONTEND_COMPILER_DEFAULT_IS_CXX11 1)
endif()

# Check the serialization component of boost has been found
if(Boost_SERIALIZATION_FOUND)
  set(ROSE_HAVE_BOOST_SERIALIZATION_LIB 1)
endif()

# TO DO: Change this to: if(Python3_FOUND)
# For now, match autoconf system
if(Python_FOUND) 
  set(ROSE_USE_PYTHON3 1)
endif()

# Append AM variable to rose-config.cfg if CMake variable is true at config time 
append_bool_config("AM_ROSE_BUILD_BINARY_ANALYSIS_SUPPORT" ROSE_BUILD_BINARY_ANALYSIS_SUPPORT)
append_bool_config("AM_ROSE_BUILD_CXX_LANGUAGE_SUPPORT" ROSE_BUILD_CXX_LANGUAGE_SUPPORT)
append_bool_config("AM_ROSE_BUILD_CUDA_LANGUAGE_SUPPORT" ROSE_BUILD_CUDA_LANGUAGE_SUPPORT)
append_bool_config("AM_ROSE_BUILD_FORTRAN_LANGUAGE_SUPPORT" ROSE_BUILD_FORTRAN_LANGUAGE_SUPPORT)
append_bool_config("AM_ROSE_BUILD_JAVA_LANGUAGE_SUPPORT" ROSE_BUILD_JAVA_LANGUAGE_SUPPORT)
append_bool_config("AM_ROSE_BUILD_PYTHON_LANGUAGE_SUPPORT" ROSE_BUILD_PYTHON_LANGUAGE_SUPPORT)
append_bool_config("AM_ROSE_BUILD_OPENCL_LANGUAGE_SUPPORT" ROSE_BUILD_OPENCL_LANGUAGE_SUPPORT)
append_bool_config("AM_ROSE_BUILD_C_LANGUAGE_SUPPORT" ROSE_BUILD_C_LANGUAGE_SUPPORT)
append_bool_config("AM_ROSE_BUILD_PHP_LANGUAGE_SUPPORT" ROSE_BUILD_PHP_LANGUAGE_SUPPORT)
append_bool_config("AM_ROSE_BUILD_ROSEHPCT" ROSE_BUILD_ROSEHPCT)
append_bool_config("AM_ROSE_EXPERIMENTAL_CSHARP_ROSE_CONNECTION" ROSE_EXPERIMENTAL_CSHARP_ROSE_CONNECTION)
append_bool_config("AM_ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION" ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION)
append_bool_config("AM_ROSE_EXPERIMENTAL_JOVIAL_ROSE_CONNECTION" ROSE_EXPERIMENTAL_JOVIAL_ROSE_CONNECTION)
append_bool_config("AM_ROSE_EXPERIMENTAL_MATLAB_ROSE_CONNECTION" ROSE_EXPERIMENTAL_MATLAB_ROSE_CONNECTION)
append_bool_config("AM_ROSE_EXPERIMENTAL_FLANG_ROSE_CONNECTION" ROSE_EXPERIMENTAL_FLANG_ROSE_CONNECTION)
append_bool_config("AM_ROSE_USE_SANITIZER" ROSE_USE_SANITIZER)
append_bool_config("AM_ROSE_EXPERIMENTAL_LIBADALANG_ROSE_CONNECTION" ROSE_EXPERIMENTAL_LIBADALANG_ROSE_CONNECTION)
append_bool_config("AM_ROSE_USING_GFORTRAN_VERSION_4_0" ROSE_USING_GFORTRAN_VERSION_4_0)
append_bool_config("AM_ROSE_USING_GFORTRAN_VERSION_4_1" ROSE_USING_GFORTRAN_VERSION_4_1)
append_bool_config("AM_ROSE_USING_GFORTRAN_VERSION_4_4" ROSE_USING_GFORTRAN_VERSION_4_4)
append_bool_config("AM_ROSE_USING_GFORTRAN_VERSION_4_5" ROSE_USING_GFORTRAN_VERSION_4_5)
append_bool_config("AM_ROSE_USING_GFORTRAN_VERSION_LATER_4_4" ROSE_USING_GFORTRAN_VERSION_LATER_4_4)
append_bool_config("AM_ROSE_USING_GFORTRAN_VERSION_LATER_4_5" ROSE_USING_GFORTRAN_VERSION_LATER_4_5)
append_bool_config("AM_ROSE_USING_BOOST_VERSION_1_35" ROSE_USING_BOOST_VERSION_1_35)
append_bool_config("AM_ROSE_USING_BOOST_VERSION_1_57" ROSE_USING_BOOST_VERSION_1_57)
append_bool_config("AM_ROSE_USING_BOOST_VERSION_1_62" ROSE_USING_BOOST_VERSION_1_62)
append_bool_config("AM_ROSE_USE_EDG_VERSION_5_0" ROSE_USE_EDG_VERSION_5_0)
append_bool_config("AM_ROSE_USE_EDG_VERSION_6_0" ROSE_USE_EDG_VERSION_6_0)
append_bool_config("AM_ROSE_USE_LONG_MAKE_CHECK_RULE" ROSE_USE_LONG_MAKE_CHECK_RULE)
append_bool_config("AM_ROSE_BUILD_OS_IS_CYGWIN" ROSE_BUILD_OS_IS_CYGWIN)
append_bool_config("AM_ROSE_USE_INTEL_PIN" ROSE_USE_INTEL_PIN)
append_bool_config("AM_ROSE_USE_SQLITE_DATABASE" ROSE_USE_SQLITE_DATABASE)
append_bool_config("AM_ROSE_BUILD_OS_IS_OSX" ROSE_BUILD_OS_IS_OSX)
append_bool_config("AM_ROSE_USE_CLANG_FRONTEND" ROSE_USE_CLANG_FRONTEND)
append_bool_config("AM_ROSE_USE_INSURE" ROSE_USE_INSURE)
append_bool_config("AM_ROSE_USING_ALTERNATE_BACKEND_C_COMPILER" ROSE_USING_ALTERNATE_BACKEND_C_COMPILER)
append_bool_config("AM_USING_XTENSA_BACKEND_COMPILER" USING_XTENSA_BACKEND_COMPILER)
append_bool_config("AM_ROSE_FRONTEND_COMPILER_DEFAULT_IS_CXX11" ROSE_FRONTEND_COMPILER_DEFAULT_IS_CXX11)
append_bool_config("AM_ROSE_USE_VXWORKS" ROSE_USE_VXWORKS)
append_bool_config("AM_ROSE_USE_GNAT" ROSE_USE_GNAT)
append_bool_config("AM_ROSE_USE_MICROSOFT_EXTENSIONS" ROSE_USE_MICROSOFT_EXTENSIONS)
append_bool_config("AM_ROSE_USE_WINDOWS_ANALYSIS_SUPPORT" ROSE_USE_WINDOWS_ANALYSIS_SUPPORT)
append_bool_config("AM_ROSE_USE_LCOV" ROSE_USE_LCOV)
append_bool_config("AM_USE_VALGRIND" USE_VALGRIND)
append_bool_config("AM_USING_GCC_3_4_4_BACKEND_COMPILER" USING_GCC_3_4_4_BACKEND_COMPILER)
append_bool_config("AM_ROSE_WITH_ATERM" ROSE_WITH_ATERM)
append_bool_config("AM_ROSE_DEBUG_NEW_EDG_ROSE_CONNECTION" ROSE_DEBUG_NEW_EDG_ROSE_CONNECTION)
append_bool_config("AM_ROSE_USE_INTERNAL_FRONTEND_DEVELOPMENT" ROSE_USE_INTERNAL_FRONTEND_DEVELOPMENT)
append_bool_config("AM_AXXPXX_SPECIFIC_TESTS" AXXPXX_SPECIFIC_TESTS)
append_bool_config("AM_HAS_LIBRARY_CURL" HAS_LIBRARY_CURL)
append_bool_config("AM_ROSE_USE_INDENT" ROSE_USE_INDENT)
append_bool_config("AM_ROSE_HAVE_YICES" ROSE_HAVE_YICES)
append_bool_config("AM_WITH_GOMP_OPENMP_LIB" WITH_GOMP_OPENMP_LIB)
append_bool_config("AM_ROSE_USE_OMNI_OPENMP" ROSE_USE_OMNI_OPENMP)
append_bool_config("AM_ROSE_USING_ROSE_CODE_GENERATION" ROSE_USING_ROSE_CODE_GENERATION)
append_bool_config("AM_ROSE_HAVE_BOOST_SERIALIZATION_LIB" ROSE_HAVE_BOOST_SERIALIZATION_LIB)
append_bool_config("AM_ROSE_USE_PYTHON3" ROSE_USE_PYTHON3)
append_bool_config("AM_ROSE_WITH_SPOT" ROSE_WITH_SPOT)

# MacOS specific check
append_bool_config("AM_OS_MACOSX" OS_MACOSX)

# Linux RHEL distribution checks
append_bool_config("AM_OS_VENDOR_REDHAT_REL5" OS_VENDOR_REDHAT_REL5)
append_bool_config("AM_OS_VENDOR_REDHAT_REL6" OS_VENDOR_REDHAT_REL6)
append_bool_config("AM_OS_VENDOR_DEBIAN" OS_VENDOR_DEBIAN)

# TODO: Add Windows macro check here 

# GCC version checks (May not need these) 
append_bool_config("AM_ROSE_USING_GCC_VERSION_LATER_4_4" ROSE_USING_GCC_VERSION_LATER_4_4)
append_bool_config("AM_ROSE_USING_GCC_VERSION_LATER_4_8" ROSE_USING_GCC_VERSION_LATER_4_8)
append_bool_config("AM_ROSE_USING_GCC_VERSION_LATER_4_9" ROSE_USING_GCC_VERSION_LATER_4_9)
append_bool_config("AM_ROSE_USING_GCC_VERSION_LATER_5_1" ROSE_USING_GCC_VERSION_LATER_5_1)
append_bool_config("AM_ROSE_USING_GCC_VERSION_LATER_6_0" ROSE_USING_GCC_VERSION_LATER_6_0)
append_bool_config("AM_ROSE_USING_GCC_VERSION_LATER_6_1" ROSE_USING_GCC_VERSION_LATER_6_1)
append_bool_config("AM_ROSE_USING_GCC_VERSION_LATER_10_1" ROSE_USING_GCC_VERSION_LATER_10_1)
append_bool_config("AM_ROSE_USING_GCC_VERSION_LATER_10_5" ROSE_USING_GCC_VERSION_LATER_10_5)

# Additional compiler checks
string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "Intel" USING_INTEL_COMPILER)
string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "GNU"   USING_GNU_COMPILER)
string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "Clang" USING_CLANG_COMPILER)

append_bool_config("AM_USING_INTEL_COMPILER" USING_INTEL_COMPILER)
append_bool_config("AM_USING_GNU_COMPILER"   USING_GNU_COMPILER)
append_bool_config("AM_USING_CLANG_COMPILER" USING_CLANG_COMPILER)

# We can add more configuration checks as needed (Boost, Z3, CUDA, etc.)
append_bool_config("AM_ROSE_HAVE_Z3" ROSE_HAVE_Z3)
append_bool_config("AM_ROSE_HAVE_LIBZ3" ROSE_HAVE_LIBZ3)
append_bool_config("AM_USE_CUDA" USE_CUDA)
append_bool_config("AM_ROSE_USE_SSL_SUPPORT" ROSE_USE_SSL_SUPPORT)
append_bool_config("AM_ROSE_HAVE_LIBDWARF" ROSE_HAVE_LIBDWARF)

# Print 
if(CMAKE_BUILD_TYPE STREQUAL "Debug") 
  message(STATUS "Added new variables to: ${ROSE_CONFIG_FILE}")
  message(STATUS "Total New variables processed: ${TOTAL_COUNT}")
  message(STATUS "Total New variables that were written: ${TRUE_COUNT}")
  message(STATUS "False variables: ${FALSE_COUNT}")
endif() 
