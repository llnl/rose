#!/usr/bin/perl

=head1 SYNOPSIS

builtinLlvmFunctions.pl [switches] files...

builtinLlvmFunctions.pl [switches] < file

=head1 DESCRIPTION

This script reads the LLVM Builtins.def file (specified on the command-line or provided as standard input) and produces a
C++ .h file on standard output. The generated .h file contains declarations for the built-in functions.

=head1 SWITCHES

=over

=item --constexpr=filename

Obtain a list of function names from the specified file. These are the functions that should have "constexpr" added
to their return type. The functions should be listed one per line. Lines whose first non-space character is '#' are
ignored.

=back

=cut


use strict;
my $status = 0;                 # exit status

########################################################################################################################
# Parse a string that contains information about the function return type and argument types. The LLVM Builtins.def file
# has a comment near the top with this information, although not all of these types appear in the Builtins.def file:
#
#     +------------------------------------------------------------------------------+
#     | v -> void                                                                    |
#     | b -> boolean                                                                 |
#     | c -> char                                                                    |
#     | s -> short                                                                   |
#     | i -> int                                                                     |
#     | h -> _Float16                                                                |
#     | f -> float                                                                   |
#     | d -> double                                                                  |
#     | z -> size_t                                                                  |
#     | F -> constant CFString                                                       |
#     | G -> id                                                                      |
#     | H -> SEL                                                                     |
#     | a -> __builtin_va_list                                                       |
#     | A -> "reference" to __builtin_va_list                                        |
#     | V -> Vector, following num elements and a base type.                         |
#     | X -> _Complex, followed by the base type.                                    |
#     | P -> FILE                                                                    |
#     | J -> jmp_buf                                                                 |
#     | SJ -> sigjmp_buf                                                             |
#     | . -> "...".  This may only occur at the end of the function list.            |
#     |                                                                              |
#     |Types may be prefixed with the following modifiers:                           |
#     | L   -> long (e.g. Li for 'long int')                                         |
#     | LL  -> long long                                                             |
#     | LLL -> 128-bit (e.g. LLLi for '__int128_t' or LLLd for '_Float128'           |
#     | S   -> signed                                                                |
#     | U   -> unsigned                                                              |
#     | I   -> Required to constant fold to an integer constant expression.          |
#     |                                                                              |
#     |Types may be postfixed with the following modifiers:                          |
#     |* -> pointer (optionally followed by an address space number)                 |
#     |& -> reference (optionally followed by an address space number)               |
#     |C -> const                                                                    |
#     |D -> volatile                                                                 |
#     +------------------------------------------------------------------------------+
#
# More details:
#   1. The _Complex ("X") is part of the following type, as in "_Complex double"
#   2. CFString, as far as I can tell, means "Core Foundation String" and is part of Mac OS X and IOS. Apple's
#      documentation for __builtin__CFStringMakeConstantString says it returns "const void*". However, using it
#      in a clang++ input gives an error message saying the return type is "const NSConstantString*", as in:
#        xx.C:2:8: error: cannot initialize a variable of type 'int' with an rvalue of type 'const NSConstantString *'
#        int x = __builtin___CFStringMakeConstantString("");
#      Adding "-fconstant-string-class=NSConstantString" to the clang++ command-line doesn't help. Neither does
#      CFConstantString or CFString work.

sub parseTypes {
    local($_) = @_;             # a string containing the return type and argument type specifications
    my(%translation) = (v => 'void', b => 'boolean', c => 'char', s => 'short', i => 'int', f => 'float',
                        d => 'double', z => 'size_t', h => '_Float16', F => 'NSConstantString',
                        a => '__builtin_va_list', A => '__builtin_va_list&', X => '_Complex',
                        P => 'FILE', J => 'jmp_buf', '.' => '...',
                        L => 'long', S => 'signed', U => 'unsigned', I => '',
                        '*' => '*', '&' => '&', C => 'const', D => 'volatile',
                        ',' => ',');

    # Split the types into a comma-separated list
    s/([LSUI]*[vbcsihfdzFGHaAVXPJ.][*&CD]*)/$1,/g;

    # Expand the letters into type names
    $_ = join " ", map { exists $translation{$_} ? $translation{$_} : "ERROR('$_')" } split //;
    s/long long long int/__int128_t/g;
    s/long long long double/_Float128/g;
    s/signed jmp_buf/sigjmp_buf/g;
    s/(_Complex)\s*,\s*([^,]*)/$1 $2/g;
    return grep {$_} split /,/;
}

########################################################################################################################
# Parse a string that contains information about function attributes. The LLVM Builtins.def file has the following
# information about this string:
#     +------------------------------------------------------------------------------+
#     | The third value provided to the macro specifies information about attributes |
#     | of the function.  These must be kept in sync with the predicates in the      |
#     | Builtin::Context class.  Currently we have:                                  |
#     |  n -> nothrow                                                                |
#     |  r -> noreturn                                                               |
#     |  c -> const                                                                  |
#     |  t -> signature is meaningless, use custom typechecking                      |
#     |  F -> this is a libc/libm function with a '__builtin_' prefix added.         |
#     |  f -> this is a libc/libm function without the '__builtin_' prefix. It can   |
#     |       be followed by ':headername:' to state which header this function      |
#     |       comes from.                                                            |
#     |  p:N: -> this is a printf-like function whose Nth argument is the format     |
#     |          string.                                                             |
#     |  P:N: -> similar to the p:N: attribute, but the function is like vprintf     |
#     |          in that it accepts its arguments as a va_list rather than           |
#     |          through an ellipsis                                                 |
#     |  s:N: -> this is a scanf-like function whose Nth argument is the format      |
#     |          string.                                                             |
#     |  S:N: -> similar to the s:N: attribute, but the function is like vscanf      |
#     |          in that it accepts its arguments as a va_list rather than           |
#     |          through an ellipsis                                                 |
#     |  e -> const, but only when -fmath-errno=0                                    |
#     |  FIXME: gcc has nonnull                                                      |
#     +------------------------------------------------------------------------------+
#
sub parseAttributes {
    local($_) = @_;
    my(%translation) = ( n => 'nothrow', r => 'noreturn', c => 'const',
                         t => '', F => '', f => '', e => '',
                         p => 'format(printf,FORMAT,FORMAT_PLUS_1)',
                         P => 'format(printf,FORMAT,0)',
                         s => 'format(scanf,FORMAT,FORMAT_PLUS_1)',
                         S => 'format(scanf,FORMAT,0)',
                         ','=>',', ':'=>':', 0=>0, 1=>1, 2=>2, 3=>3, 4=>4, 5=>5, 6=>6, 7=>7, 8=>8, 9=>9 );

    s/([nrctFfe]|[pPsS]:\d+:)/$1,/g; # split individual specs into a comma-separated list

    # Translate the letters into __attribute__ arguments
    $_ = join " ", map { exists $translation{$_} ? $translation{$_} : "ERROR('$_')" } split //;
    s/FORMAT,FORMAT_PLUS_1\)\s*:\s*(\d+)\s*:\s*/"$1,".($1+1).")"/ge;
    s/FORMAT,0\)\s*:\s*(\d+)\s*:\s*/$1,0)/g;

    # Create the entire __attribute__((...)) string or the empty string
    my($attr_args) = join ", ", grep {$_} split /\s*,\s*/;
    return "" unless $attr_args;
    return "__attribute__(($attr_args))";
}

########################################################################################################################
# Comments at the top and bottom of this file.
sub commentary {
    print "//  DO NOT EDIT THIS FILE -- IT WAS GENERATED AUTOMATICALLY\n";
    print "//\n";
    my($year,$month,$mday) = (localtime)[5,4,3];
    printf "// Generated on %04d-%02d-%02d by %s\n// using %s\n", $year+1900, $month+1, $mday, $ENV{USER}, $0;
}

########################################################################################################################
# Process command-line
my %constexprs;
while (@ARGV && $ARGV[0] =~ /^-/) {
    my($arg) = shift @ARGV;
    if ($arg eq '--') {
	last;
    } elsif ($arg =~ /^--constexpr=(.+)/) {
	# Read the file that contains a list of function names that need "constexpr" added to their return type.
	open F, "<", $1 or die "cannot open $1: $!\n";
	while (<F>) {
	    if (/^\s*([a-zA-Z_]\w*)\s*$/ && substr($1,0,1) ne "#") {
		$constexprs{$1} = "ROSE_CONSTEXPR";
	    }
	}
	close F;
    } elsif ($arg =~ /^-/) {
	die "unknown argument: $arg\n";
    }
}

########################################################################################################################
commentary;
print "\n" x 10;
while (<>) {
    if (my($fname,$argspec,$attrspec) = /^\s*BUILTIN\s*\(\s*(\w+)\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"\s*\)/) {
        my($retval,@argtypes) = parseTypes $argspec;
        my($attribute) = parseAttributes $attrspec;
        if (my($letter) = join("",$retval,@argtypes) =~ /ERROR\('(.*?)'\)/) {
            print STDERR "$0: error: unrecognized return/argument type specification '$letter'\n    at $ARGV:$.: $_";
            $status = 1;
        } elsif (my($letter) = $attribute =~ /ERROR\('(.*?)'\)/) {
            print STDERR "$0: error: unrecognized attribute type specification '$letter'\n    at $ARGV:$.: $_";
            $status = 1;
        } else {
            my($decl) = "$constexprs{$fname} $retval $fname(" . join(", ", @argtypes) . ") $attribute;";
            # This cleanup is entirely optional, but it makes the output look nicer.
            $decl =~ s/\s{2,}/ /g;                 # single spaces only
            $decl =~ s/\s*(\W)\s*/$1/g;            # remove spacing around non-word tokens
            $decl =~ s/,/, /g;                     # add spaces after commas
            $decl =~ s/([)*])(\w)/$1 $2/g;         # space between certain things
	    $decl =~ s/^\s+//;			   # remove space at beginning of line
            print $decl, "\n";
        }
    } elsif (/^\s*BUILTIN\b/) {
        print STDERR "$0: error: did not recognize BUILTIN specification\n    at $ARGV:$.: $_";
        $status = 1;
    }
}
print "\n" x 10;
commentary;

exit $status;
