#!/bin/sh

set -e

# Create the initial version of libneeded.so, which will be a DT_NEEDED
# dependency of 'prog'. This version provides definitions of both abc()
# and xyz().

cc -DINCL_ABC -DINCL_XYZ -Wall -fPIC -DLIB='"libneeded"' -c lib.c
cc -shared -o libneeded.so lib.o

# Create a library, libdlopen.so that will be opened with dlopen(). This
# library provides definitions of both abc() and xyz().

cc -DINCL_ABC -DINCL_XYZ -Wall -fPIC -DLIB='"libdlopen"' -c lib.c
cc -shared -o libdlopen.so lib.o

set +e

echo DONE

echo
echo "====="
echo "DT_NEEDED library that provides abc() and xyz()"
echo

# Create the executable with libneeded.so as a DT_NEEDED dependency;
# the linker is satisfied, since this version of libneeded.so provides
# definitions of abc() and xyz().

cc -o prog prog.c libneeded.so

LD_LIBRARY_PATH=. ./prog

echo
echo "====="
echo "DT_NEEDED library that provides abc() but not xyz() (error expected)"
echo

# Create a new version of libneeded.so that provides a definition of abc()
# but no definition of xyz().

cc -DINCL_ABC -Wall -fPIC -DLIB='"libneeded"' -c lib.c
cc -shared -o libneeded.so lib.o

LD_LIBRARY_PATH=. ./prog

echo
echo "====="
echo "DT_NEEDED library that provides abc() + dlopen(RTLD_GLOBAL) of library that provides xyz()"
echo

# This is the test case of main interest...

LD_LIBRARY_PATH=. ./prog ./libdlopen.so

echo
echo "====="
echo "DT_NEEDED library that provides abc() + dlopen(RTLD_GLOBAL) of library that provides xyz() + LD_BIND_NOW set (error expected)"
echo

LD_BIND_NOW=y LD_LIBRARY_PATH=. ./prog ./libdlopen.so

echo
echo "====="
echo "DT_NEEDED library that provides abc() + dlopen(RTLD_LOCAL) of library that provides xyz() (error expected)"
echo

LD_LIBRARY_PATH=. ./prog ./libdlopen.so x
