#!/usr/bin/perl
# DO NOT DISABLE without first checking with a ROSE core developer
BEGIN {push @INC, $1 if $0 =~ /(.*)\//}
use strict;
use FileLister;
use Policies;

my $desc = <<EOF;
The \$ROSE/src/Rose directory is somewhat more structured than many other
parts of ROSE. It represents the ::Rose namespace and everything it contains.

Currently, some parts of the ::Rose namespace exist in other source directories,
and our goal is to gradually move them to a directory hierarchy that mirrors the
namespace hierarchy. As files are moved into the \$ROSE/src/Rose directory
hierarchy, they must also be modified to adhere to the policies listed here.
EOF

# The following is a list of policies that we are enforcing for everything
# under the new $ROSE/src/Rose directory.
my @policies = (

    #------------------------------------------------------------------------
    {
        name => 'namespace_capitalization',

        policy =>
        'A namespace MUST be capitalized as PascalCase, where the first letter
        of each word is capitalized and the remaining letters are lower-case.
        Abbreviations and acronyms (when allowed) are treated as words except
        that the following acronyms MUST be all caps when they stand alone: API,
        AST, CFG, and IO.',

        reason =>
        'Consistent capitalization means that the user has to consult
        documentation less frequently to figure out how to spell things. The
        reason acronyms are capitalized as if they are words is because things
        like "AstToPdf" and "CfgCsvIo" are generally easier to read than
        "ASTToPDF" and "CFGCSVIO". The exception for the few stand alone
        acronyms is because they\'re usually spelled that way already in ROSE
        and occur frequently.'
    },

    #------------------------------------------------------------------------
    {
        name => 'english_words',

        policy =>
        'The individual words componsing a PascalCase symbol MUST be standard
        English words, words created consistently from standard English words,
        or a list of exceptional words. You can find the exceptional words in
        the script that impliments this policy. The policy also has a list of
        words that MUST NOT be used. Any changes to these lists should be
        approved at a ROSE developers\' meeting.',

        reason =>
        'This policy attempts to avoid a proliferation of mixing of full words
        and their abbreviations or acronyms since lack of consistent spelling
        would force users to consult the documentation more often to find out
        how to spell things. Users and ROSE authors are free to create their own
        private aliases if they\'re burdened by increased symbol lengths.'
    },

    #------------------------------------------------------------------------
    {
        name => 'namespace_header_exists',

        policy =>
        'For every namespace (e.g., Rose::Foo::Bar), there MUST be a header file
        whose name mirrors the namespace (e.g., <Rose/Foo/Bar.h>. This is called
        the "namespace header". A temporary exception to this rule is that there
        is no <Rose.h> since it would be confusing due to already having a
        <rose.h> that serves a slightly different purpose.',

        reason =>
        'This is so that users know which file they need to include in order
        to use that namespace.'
    },

    #------------------------------------------------------------------------
    {
        # FIXME: untested
        name => 'namespace_directory_exists',

        policy =>
        'If a namespace has more than one header file, or more than one
        compilation unit, or it has sub-namespaces, then it MUST have its own
        directory (e.g., $ROSE/src/Rose/Foo/Bar) that contains all files related
        to the namespace (other than the "namespace header" mentioned in another
        policy). This directory is called the "namespace directory". A namespace
        that doesn\'t meet this condition MAY either have its own namespace
        directory or place its one or two source files in the directory of its
        parent namespace.',

        reason =>
        'Placing source files in a directory is a form of encapsulation, with
        all its benefits, and makes the directory hierarchy mirror the namespace
        hierarchy. It also eliminates the need to prefix its files with a common
        string to avoid potential clashes with other files.'
    },

    #------------------------------------------------------------------------
    {
        name => 'doxygen_input',

        policy =>
        'Every directory that contains a public header file MUST be listed as a
        Doxygen input, and all directories listed as Doxygen input MUST contain
        at least one header.',

        reason =>
        'Namespaces are useless if they\'re not documented, and Doxygen will not
        pick up the documentation if the directory is not listed as an input. We
        also want to make sure that our Doxygen configuration stays clean by not
        listing directories that have been removed (or misspelled).'
    },

    #------------------------------------------------------------------------
    {
        name => 'namespace_header_includes_all',

        policy =>
        'If a namespace has a namespace directory, then the namespace header
        (located in the directory of the parent namespace) MUST #include all
        other public headers associated with the namespace (e.g.,
        <Rose/Foo/Bar.h> must #include all the headers with names like
        <Rose/Foo/Bar/*.h>). A temporary exception to this rule is that there is
        no <Rose.h> file that #include\'s everything for the ::Rose namespace.',

        reason =>
        'This permits users to obtain with a single #include all declarations
        that are necessary in order to use a particular namespace. This
        addresses the concern that having finer granularity header files will
        cause a burden to the user by increasing the number of
        #include directives they need.'
    },

    #------------------------------------------------------------------------
    {
        name => 'h_extension',

        policy =>
        'Every public header file MUST use the extension ".h".',

        reason =>
        'This consistency permits users to spell header files correctly without
        having to consult the documentation. Also, by avoiding uncommon
        extensions, it increases the chance that a user\'s editor will properly
        recognize the file as containing source code.'
    },

    #------------------------------------------------------------------------
    {
        name => 'include_by_angle_brackets',

        policy =>
        'All #include directives MUST use angle brackets instead of quotes.',

        reason =>
	'The #include search paths are implementation defined. The standard
	says that the quoted variety does extra things compared to the angle
	bracket variety. C++ compilers do an assortment of extra things for the
	quoted version, some of which are quite compilicated (e.g., MSVC).
        A side effect of requiring bracket-includes is that we can check at
	compile time that ROSE headers always include other ROSE headers by
 	using file paths that are relative to the installed "include" directory.
	That is, we want to include Rose/BinaryAnalysis/Architecture/Powerpc.h
	instead of Powerpc.h, or ../Powerpc.h, or Architecture/Powerpc.h, etc.
	The former makes it clear to readers that Powerpc.h is a ROSE header
	that defines something in the Rose::BinaryAnalysis::Architecture
	namespace.'
    },

    #------------------------------------------------------------------------
    {
        name => 'include_once',

        policy =>
        'Every header file MUST be protected from multiple inclusion of the form
        "#ifndef SYMBOL" followed by "#define SYMBOL".',

        reason =>
        'This permits the header to be safely and efficiently included multiple
        times directly and/or indirectly. Do not use "#pragma once" because it
        is not portable.'
    },

    #------------------------------------------------------------------------
    {
        name => 'once_symbol',

        policy =>
        'The C preprocessor symbol used for multiple inclusion protection MUST
        be of the form "ROSE_x_H" where "x" is formed by taking the fully
        qualified namespace, removing the leading "Rose" component, and
        replacing each component separator ("::") with an underscore
        ("_"). Capitalization of the components of "x" must match capitalization
        of the namespace components.',

        reason =>
        'This enables users to test whether a certain ROSE capability is present
        in their installed version of ROSE by testing whether a particular
        header has been included indirectly. For instance, the user might
        include <Rose/BinaryAnalysis/Disassembler.h> and then check whether
        ROSE_BinaryAnalysis_Disassembler_Jvm_H is defined in order to determine
        if this version of ROSE supports analysis of Java class files. The
        reason for "ROSE" being allcaps is because we have a separate policy
        that states that all ROSE-related C preprocessor symbols begin with
        "ROSE_".'
    },

    #------------------------------------------------------------------------
    {
        name => 'unique_file_names',

        policy =>
        'A single directory cannot contain two or more files that have the same
        name when converted to all lower case.',

        reason =>
        'The often used file system configuration on OS X is not POSIX conformant
        and is thus unable to handle some situations where two files have names
        that differ only in the case of some letters.

        By the way, it is fine for header files in two different directories to
        have the same name because all headers under the "Rose" namespace are
        included with their directory, as in "#include <Rose/...>".'
    },

    #------------------------------------------------------------------------
    {
        name => 'first_include',

	policy =>
	'The first #include must be either RoseFirst.h or some other Rose/*.h
	header file.',

	reason =>
	'Not all system header files are deterministic, and RoseFirst.h defines
	some CPP symbols that affect the behavior of including those headers. If
	we allowed those non-deterministic headers to be included both with and
	without the proper configuration into various translation units, then we
 	run the risk of violating the C++ one definition rule (ODR).'
    },
);

# Policies indexed by name
my %policies;
$policies{$_->{name}} = $_ for @policies;

###############################################################################################################################
# List of ROSE words that don't appear in the dictionary.
#
# DO NOT ADD ABBREVIATIONS TO THIS LIST UNLESS THEY ARE ALREADY ROUTINELY USED IN ROSE.
#
# Abbreviations are discouraged because we want to avoid having the situation where some ROSE authors use the abbreviation and
# others don't, resulting in a situation where users never know whether to use the full word or the abbreviation! Don't worry
# much about having to type long names -- that's why C++ has type aliasing and 'using' statements that allow you and users to
# use whatever abbreviation you and they want in order to save typing.
#
# By the way, be careful what you put in the "comments" because they're not actually comments. The words in the comments will
# be added to the extra words list. Case is not important in this list (use any combination of upper and lower case)
my @extra_words = qw/
    AARCH32                     # ARM architecture
    AARCH64                     # ARM architecture
    Asm                         # Assembly
    AST                         # Abstract Syntax Tree
    CFG                         # Control Flow Graph
    CIL                         # Common Intermediate Language
    ColdFire                    # Microprocessor
    GDB                         # GNU Debugger
    JVM                         # Java Virtual Machine
    callback                    # executable code passed as an argument
    callbacks                   # list of executable code passed as an argument
    Concolic                    # as in Concolic Testing
    demangler                   # missing from dictionary
    dereference                 # act of obtaining a pointee from a pointer
    enum
    enums
    hexdump			# the name of an established Unix tool reimplemented in ROSE
    i386                        # Intel 80386
    Itanium                     # C++ ABI named after Intel's Itanium architecture
    linux                       # an operating system kernel
    LLVM                        # the compiler technology
    M68k                        # Motorola 68000
    NXP                         # Microprocessor manufacturer
    partitioner                 # missing from dictionary
    Pentium                     # Intel processor
    PentiumII                   # Intel processor
    PentiumIII                  # Intel processor
    PowerPC                     # a computer architecture
    reachability                # missing from dictionary
    RISC                        # Reduced Instruction Set Computer
    SARIF                       # Static Analysis Results Interchange Format, a Microsoft standard
    SMT                         # symmetric modulo theory
    SMTLIB                      # a language for SMT solvers
    SRecord                     # Motoroal S-Record format
    SValue                      # semantic value
    traversal                   # missing from dictionary
    unparser                    # missing from dictionary
    VxWorks                     # real time operating system from Wind River Systems
    X86                         # Intel 8086 family
    YAML                        # Yet Another Markup Language
    Z3                          # a particular SMT solver
    /;

###############################################################################################################################
# This is a list of abbreviations we want to avoid. The left hand column must be spelled all lower case. The right hand column
# is the PascalCase replacement that should be used.
my %bad_words = (
    # Bad word              Replacement           What already uses the replacement that may cause user confusion
    #==================     ====================  ================================================================
    'cmdline'           => 'CommandLine',         # Rose::CommandLine etc.
    'config'            => 'Configuration',       # Rose::BinaryAnalysis::Partitioner2::Configuration, ::Configuration, etc.
    'deref'             => 'Dereference',         # CodeThorn, RTED, Fuse, C++ backend, etc.
    'expr'              => 'Expression',          # SgExpression, SgAsmBinaryExpression, SgAwaitExpression, etc.
    'oob'               => 'OutOfBounds',         # uncommon abbreviation
    'uninit'            => 'Uninitialized',       # CodeThorn, Compass, ROSETTA-generated code, Flang frontend, generic data-flow, etc.
    'util'              => 'Utility',             # StringUtility, etc.
    'utils'             => 'Utility',             # StringUtility, etc.
    'utilities'         => 'Utility',             # StringUtility, etc.
    );




###############################################################################################################################
# Returns a severity prefix string appropriate for whether the specified policy is enabled or disabled for the file. Additional
# information string is the second return value (such as responsible party for warnings).
# See also, the regular expression in `countViolations` below.
sub severity {
    my($file_name, $policy_name) = @_;
    my($disabled,$info) = is_disabled_in_dir($file_name, $policy_name);
    my($w) = $disabled ? "warning: " : "error: ";

    my($extra) = "";
    for my $key (keys %$info) {
        $extra .= "\n" . $key . ": " . $info->{$key} unless $key eq 'disabled';
    }
    return ($w, $extra);
}

###############################################################################################################################
# True if the specified header is deprecated using the approved deprecation mechanism.
sub headerIsDeprecated {
    my($filename) = @_;
    my($isDeprecated);
    open HEADER, "<$filename" or return;
    while (<HEADER>) {
        if (/ROSE_PRAGMA_MESSAGE\("This header is deprecated/) {
            $isDeprecated = 1;
            last;
        }
    }
    close HEADER;
    return $isDeprecated;
}

###############################################################################################################################
# The symbol used to protect this header from mutliple inclusion. The return value is a pair consisting of the #ifndef symbol
# and the following #define symbol.
sub multiInclusionProtectionSymbols {
    my($filename) = @_;
    open HEADER, "<$filename" or return;
    while (<HEADER>) {
        if (/^\s*#\s*ifndef\s+(\w+)/) {
            my $ifndef = $1;
            while (<HEADER>) {
                if (/^\s*#\s*define\s+(\w+)/) {
                    my $def = $1;
                    close HEADER;
                    return ($ifndef, $def);
                }
            }
            close HEADER;
            return ($ifndef, undef);
        }
    }
    close HEADER;
    return ();
}

###############################################################################################################################
# Read a file and find the name of the first #include file, if any.
sub findFirstInclude {
    my($filename) = @_;
    open HEADER, "<$filename" or return;
    while (<HEADER>) {
	if (/^\s*#\s*include\s*["<]([^">]+)[">]/) {
	    close HEADER;
	    return $1;
	}
    }
    close HEADER;
    return ();
}

###############################################################################################################################
# Read the Doxygen configuration file and get a list of input directories under src/Rose.
sub doxygenInputDirectories {
    my($lister) = @_;
    my $configName = $lister->{gitdir} . "/docs/Rose/rose.cfg.in";
    my %dirs;
    open CONFIG, '<', $configName or return;
    while (<CONFIG>) {
        if (/^\s*INPUT\s*=/) {
            $dirs{$1} = 1 if /\@top_srcdir\@\/src\/(Rose\S*)/;
            while (<CONFIG>) {
                $dirs{$1} = 1 if /\@top_srcdir\@\/src\/(Rose\S*)/;
            }
            close CONFIG;
            return %dirs;
        }
    }
    close CONFIG;
    return %dirs;
}

###############################################################################################################################
# Load English words
sub loadWords {
    my %words;
    open WORDS, '</usr/share/dict/words' or return;
    while (<WORDS>) {
        chomp $_;
        $_ = lc $_;
        if (/^[a-z]+$/) {
            $words{$_} = 1;

            # Source code often makes words by adding "er" to a verb, like "loader"
            if (/e$/) {
                $words{$_ . "r"} = 2;
            } elsif (/[aeiou]([^aeiou])$/) {
                $words{$_ . $1 . "er"} = 2;
            } else {
                $words{$_ . "er"} = 2;
            }
        }
    }
    close WORDS;

    # Add special words, but only if we found any words.
    if (keys %words) {
        foreach (@extra_words) {
            $_ = lc $_;
            $words{lc $_} = 3 if /^[a-z][a-z0-9]*$/;
        }
    }

    return %words;
}

###############################################################################################################################
# Given a word that looks plural, return a singular spelling
sub depluralize {
    my($word, $word_list) = @_;

    my @possibilities;
    push @possibilities, $1."y" if $word =~ /^(.+)ies$/;
    push @possibilities, $1 if $word =~ /^(.+)s$/;
    push @possibilities, $1."ex" if $word =~ /^(.*)ices$/;

    foreach (@possibilities) {
        return $_ if exists $word_list->{$_};
    }

    return;
}

###############################################################################################################################
# Split a PascalCase, camelCase, or snake_case symbol into individual word components
sub splitSymbolIntoWords {
    my($symbol) = @_;
    my $s = $symbol;
    my @retval;
    while ($s ne "" && $s =~ /^_?([A-Z]*[a-z0-9]*)(.*)/) {
        my ($word, $rest) = (lc $1, $2);
        push @retval, $word;
        $s = $rest;
    }
    return @retval;
}

###############################################################################################################################
# Check that all parts of a symbol are words
sub checkWords {
    my($symbol, $word_list) = @_;
    my @words = splitSymbolIntoWords($symbol);
    my @errors;
    foreach (@words) {
        if (exists $bad_words{$_}) {
            push @errors, "\"$_\" should be replaced by \"$bad_words{$_}\"";
        } elsif (!exists $word_list->{$_}) {
            my($noDigits) = /^(.*?)\d+/; # check again after stripping trailing digits
            push @errors, "\"$_\" is not an English word" if !exists $word_list->{$noDigits};
        }
    }

    #my $last_word = $words[-1];
    #if (my($singular) = depluralize($last_word, $word_list)) {
    #   push @errors, "\"$last_word\" should probably not be plural";
    #}

    return @errors;
}

###############################################################################################################################
# Split the string into lines, replace all indentation with the specified string.
sub indent {
    my($indentation, @s) = @_;
    my @lines;
    foreach (@s) {
        chomp;
        foreach (split /\n/) {
            s/^\s+//;
            push @lines, $_;
        }
    }
    return "" unless @lines;
    return $indentation . join("\n" . $indentation, @lines) . "\n";
}

###############################################################################################################################
# How many violations were found. Returns the total, and the number of errors.
sub countViolations {
    my($violations) = @_;
    my $n = 0;
    my $errors = 0;
    foreach (keys %{$violations}) {
        die "invalid key $_" unless exists $policies{$_};
        $n += @{$violations->{$_}};
        $errors += scalar(grep {/^error: /} @{$violations->{$_}});
    }
    return ($n, $errors);
}

###############################################################################################################################
# Print violations and return number of errors.
sub printViolations {
    my($violations) = @_;
    my($total,$nerrors) = countViolations($violations);
    return 0 if $total == 0;

    if (1 == $total) {
        print $desc, "\nThe following violation was found:\n";
    } else {
        print $desc, "\nThe following $total violations were found:\n";
    }

    my $me = $ENV{USER} || "i am nobody";
    my $i = 0;
    my $policy_number = 0;
    foreach (@policies) {
        my $policy = $_;
        print "\n    ", '-' x 90, "\n";
        print "    Policy #", ++$policy_number, " \"", $policy->{name}, "\"\n\n";
        print indent("    ", "Requirement: " . $policy->{policy}), "\n";
        print indent("    ", "Reason: " . $policy->{reason}), "\n";
        for my $text (@{$violations->{$policy->{name}}}) {
            ++$i;
            my($highlight) = $text =~ /\Q<$me@\E/ && -t STDOUT;
            my($hl_begin) = $highlight ? "\033[30;43m" : "";
            my($hl_end) = $highlight ? "\033[0m" : "";
            my($prefix) = "        ($i) ";
            for my $line (split "\n", $text) {
                print $prefix, $hl_begin, $line, $hl_end, "\n";
                $prefix = ' ' x length($prefix);
            }
        }
        print  "        No violations found for this policy\n" if 0 == @{$violations->{$policy->{name}}};
    }
    return $nerrors;
}

###############################################################################################################################
# Main program

# Policy violations detected. The hash keys are the violation names (same as keys of %policies), and the values are each
# a list of error messages.
my %violations;
$violations{$_} = [] for keys %policies;

my $files = FileLister->new(@ARGV);
my %docdirs = doxygenInputDirectories($files);
my %words = loadWords;
my %dir_files;
while (my $filename = $files->next_file) {
    # Check for unique case-insensitive file names. We assume every file is non-unique for now and push a provisional
    # violation to $dir_files. Later, we'll copy the actual violations to $violations. The $dir_files is a hash of hashes of
    # arrays. The first key is the directory name, the second key is the lower-case file name within the directory, and the
    # array has one provisional violation string per file name.
    if ($filename =~ /^.*(\/src\/Rose.*)\/([^\/]+)$/) {
        my ($dir, $file) = ($1, $2);
        $dir_files{$dir} ||= {};
        $dir_files{$dir}{lc $file} ||= [];
        my($w,$u) = severity($filename, 'unique_file_names');
        push @{$dir_files{$dir}{lc $file}}, "${w}file \"$file\" is not case-insensitive unique within \"$dir\"${u}";
    }

    if ($filename =~ /^(.*\/src)\/(Rose\/.*)\.(h|hpp|h\+\+|H)/) {
        my ($root, $nsFile, $extension) = ($1, $2, $3);
        my @nsParts = split("/", $nsFile);
        my $ns = join("::", @nsParts);
        my $dir = join("/", @nsParts[0 .. $#nsParts-1]);
        my $note;

        # If this header is deprecated, then none of these rules apply
        next if headerIsDeprecated($filename);

        # All header files should use ".h" for their file extension
        if ($extension ne "h") {
            my($w,$u) = severity($filename, 'h_extension');
            push @{$violations{h_extension}}, "${w}file \"$filename\" must use \".h\" as its extension${u}";
        }

        # The directory must be an input for Doxygen if it contains a header file
        if (!exists $docdirs{$dir}) {
            my($w,$u) = severity($filename, 'doxygen_input');
            push @{$violations{doxygen_input}}, "${w}directory \"$root/$dir\" must be included in the list of Doxygen inputs${u}";
        }
        $docdirs{$dir} = 2;

        # Namespace symbol must be composed of valid words.
        if (keys %words) {
            if (my @badWords = checkWords($nsParts[-1], \%words)) {
                my($w,$u) = severity($filename, 'english_words');
                push @{$violations{english_words}}, map("${w}in \"$ns\", $_${u}", @badWords);
            }
        }

        # Namespace symbol must be PascalCase
        my $nsLastCase = $nsParts[-1];
        $nsLastCase =~ s/[A-Z]/X/g;
        if (($nsLastCase =~ /XXX|_|-/ || $nsLastCase eq "XX" || $nsLastCase =~ /^[^X]/) && $nsLastCase =~ /[^X]/) {
            my($w,$u) = severity($filename, 'namespace_capitalization');
            push @{$violations{namespace_capitalization}}, "${w}in \"$ns\", \"$nsParts[-1]\" must be PascalCase${u}";
        } else {
            # Check the the multi-inclusion-protection symbol is correct
            my($expectedMipSymbol) = join("_", "ROSE", @nsParts[1 .. $#nsParts], "H");
            my($ifndef,$def) = multiInclusionProtectionSymbols($filename);
            if ($ifndef eq "") {
                my($w,$u) = severity($filename, 'include_once');
                push @{$violations{include_once}},
                    "${w}file \"$filename\" must have multi-inclusion protection using #ifndef/#define${u}";
            } elsif ($ifndef ne $expectedMipSymbol) {
                my($w,$u) = severity($filename, 'once_symbol');
                push @{$violations{once_symbol}},
                    "${w}file \"$filename\" multi-inclusion symbol must be \"$expectedMipSymbol\" (not \"$ifndef\")${u}";
            } elsif ($def eq "") {
                my($w,$u) = severity($filename, 'once_symbol');
                push @{$violations{once_symbol}}, "${note}file \"$filename\" multi-inclusion lacks #define${u}";
            }
        }

        # The parent header must exist.
        my $parentHeaderExists;
        my $parentHeader = join("/", $root, @nsParts[0 .. $#nsParts-1]) . ".h";
        if (! -r $parentHeader) {
            if ($#nsParts == 1 && $nsParts[0] eq "Rose") {
                # For the time being, there is no src/Rose.h file because it would be confusing
                # to have both <rose.h> and <Rose.h>, especially on file systems that don't follow
                # POSIX specifications (like macOS).
            } else {
                my($w,$u) = severity($filename, 'namespace_header_exists');
                push @{$violations{namespace_header_exists}}, "${w}file \"$parentHeader\" must exist for ${ns}${u}";
            }
        } else {
            $parentHeaderExists = 1;
        }

        # There must be a "#include <" . $nsFile . ".h>" in the parent
        if ($parentHeaderExists) {
            my $foundInclude = 0;
            open PARENT, "<$parentHeader";
            while (<PARENT>) {
                if (my($sep, $name) = /^\s*#\s*include\s*([<"])(.*)[">]/) {
                    if ($name eq "$nsFile.h") {
                        if ($sep ne "<") {
                            my($w,$u) = severity($filename, 'include_by_angle_brackets');
                            push @{$violations{include_by_angle_brackets}},
                                "${w}in \"$parentHeader\", \"$nsFile\" must be included with <> not \"\"${u}";
                        }
                        $foundInclude++;
                    }
                }
            }
            close PARENT;
            if ($foundInclude == 0) {
                my($w,$u) = severity($filename, 'namespace_header_includes_all');
                push @{$violations{namespace_header_includes_all}},
                    "${w}file \"$parentHeader\" does not include <$nsFile.h>${u}";
            } elsif ($foundInclude > 1) {
                my($w,$u) = severity($filename, 'namespace_header_includes_all');
                push @{$violations{namespace_header_includes_all}},
                    "${w}file \"$parentHeader\" includes <$nsFile> more than once${u}";
            }
        }

	# The header file must #include featureTests.h, RoseFirst.h, or some other Rose/*.h header before including any other
	# headers (ROSE headers or system headers). This is because RoseFirst.h defines some macros that affect the behavior of some
	# non-deterministic system header files.
	my($firstInclude) = findFirstInclude($filename);
	if ($firstInclude ne "" && $firstInclude ne "featureTests.h" && $firstInclude ne "RoseFirst.h" &&
	    $firstInclude !~ /^Rose\/.*\.h$/) {
	    my($w,$u) = severity($filename, "first_include");
	    push @{$violations{first_include}},
		"${w}file \"$filename\" first #include must be <RoseFirst.h> or some other Rose/*.h header file" .
		" (not $firstInclude)${u}";
	}
    }
}

# Are there any doxygen directories that are not referenced?
foreach (keys %docdirs) {
    if ($docdirs{$_} == 1) {
        push @{$violations{doxygen_input}}, "directory \"src/$_\" is listed as a doxygen input but is not used";
    }
}

# Look at the provisional violations for unique_file_names. For any directory that contains two of more files that compare
# equal in a case insensitive manner, copy those real violations to $violations to be output later.
foreach my $dir (sort keys %dir_files) {
    foreach my $provisional_violations (values %{$dir_files{$dir}}) {
        my $n = @{$provisional_violations};
        push @{$violations{unique_file_names}}, @{$provisional_violations} if $n > 1;
    }
}

my $nerrors = printViolations(\%violations);
if ($nerrors > 0) {
    print "\n*** error: policy checks failed for the ::Rose namespace (src/Rose directory)",
        ": $nerrors ", (1==$nerrors?"error":"errors"), "\n";
    exit(1);
} else {
    print "\nNo policy checks failed for the ::Rose namespace (src/Rose directory)\n";
    exit(0);
}
