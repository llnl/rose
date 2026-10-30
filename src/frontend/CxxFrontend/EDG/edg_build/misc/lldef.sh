#!/bin/sh
#
# Used by CMake to generate the file "lldef", containing
# the definition for "-DHAVE_LONG_LONG=[0,1]".
#
# See CMakeLists.txt:generate-lldef

if test -z "$CC"; then
  echo "[FATAL] C Compiler \$CC variable is not set."
  exit 1
fi

echo "Building lldef"
rm -f lltest.c lldef

# Default value
echo "-DHAVE_LONG_LONG=0" > lldef || exit 1

cat > lltest.c <<-EOF
#include <stdio.h>
long long i = 0x12341234;
int main() { printf("%llx", i); return 0; }
EOF

if "${CC}" lltest.c > /dev/null 2>&1 ; then
  if (./a.out | fgrep "12341234") > /dev/null ; then
    echo "-DHAVE_LONG_LONG=1" > lldef ;
  else
    rm -f lldef
    echo "[FATAL] Error during lltest.c compile test in $0"
    exit 1
  fi
else
  echo "[FATAL] Error compiling test code in $0"
  exit 1
fi

rm -f lltest.c a.out
