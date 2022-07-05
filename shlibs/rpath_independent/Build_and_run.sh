#!/bin/sh
#
# Build and run the code.
#
# If a command-line argument was supplied, do not build libx.so with an rpath.
# As a consequence, the dynamic linker won't be able to find liby.so at run
# time.

mkdir -p sub

dtags="-Wl,--enable-new-dtags"		# Use DT_RUNPATH rpaths

# To investigate the (different) behavior of DT_RPATH, change the above to:
#
#       dtags="-Wl,--disable-new-dtags"

if test $# -gt 0; then
    libx_opt=""
    echo "Building libx.so without an rpath"
else
    libx_rpath_opt='-Wl,-rpath,$ORIGIN'
    echo "Building libx.so with an rpath"
fi

echo

cc -fPIC -g -shared -o liby.so mody.c
cc -fPIC -g -shared -o libx.so modx.c $libx_rpath_opt $dtags liby.so

cc -o prog prog.c -Wl,--rpath-link,sub -Wl,-rpath,'$ORIGIN/sub' $dtags libx.so

echo "Dependencies of prog:"
objdump -p prog | grep NEEDED
echo
echo "Dependencies of libx.so:"
objdump -p libx.so | grep NEEDED

mkdir -p sub
mv libx.so sub
mv liby.so sub

echo
echo "File layout:"
find prog sub -type f | sed 's/^/        /'

set -v

objdump -p prog | grep "R[UN]*PATH"
objdump -p sub/libx.so | grep "R[UN]*PATH"

./prog
