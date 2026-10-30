#!/bin/sh
#
# Used to generate "dettarg.out".
#
# See CMakeLists.txt:dettarg.out

: ${BINDIR:=$1}

echo "Building dettarg.out"

"${BINDIR}/dettarg" > dettarg.out || exit 1

if fgrep "HAVE_LONG_LONG=0" lldef > /dev/null ; then
  echo "#define INTEGER_VALUE_REPR_IS_A_HOST_INTEGER 0" >> dettarg.out || exit 1
else
  if fgrep "HAVE_LONG_LONG=1" lldef > /dev/null ; then
    : echo "[INFO] Compiler supports long long"
  else
    rm -f dettarg.out
    echo "[FATAL] Unknown error during fgrep"
    exit 1
  fi
fi
