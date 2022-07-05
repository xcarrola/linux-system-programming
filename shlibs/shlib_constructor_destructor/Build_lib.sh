#!/bin/bash

# This script creates source files and builds shared libraries from those
# files. It creates one source file and library per command-line argument.
# Arguments have the general form:
#
#       suffix[@ref][:dep[,...]
#
#       Examples: z1 z2:z1 rx@x1
#
# This creates a source file named lib_$suffix.c and a library lib_$suffix.so.
# The library will contain a function named $suffix_func() as well as
# constructor and destructor functions.
#
# If 'dep,...' is supplied, the library will be linked so that it depends
# on lib_{dep}.so, for all of the listed dependencies.
#
# If @ref is supplied, then the library will include a function that
# has a reference to $ref_func(), which should be a reference to a
# $suffix_func() in another library.
#
# ====================
#
# emit_source()
#
# Arguments:
#	$1 -- libname
#	$2 -- extref
#
# Emit a C source file that can be used to build a shared library.
# The source file is called lib_${libname}.c, and contains the following
# functions:
#
# ${libname}_func()
#	A function with a unique name in this source file that may be referred
#       to from another library (see testref() below)
# ${libname}_ctor()
#       A constructor function that is called when this shared library is
#	loaded into memory.
# ${libname}_dtor()
#       A destructor function that is called when this shared library is
#	unloaded from memory.
# testref() [optional, only if a second argument is provided]
#	A function that contains a reference to ${testref}_func()

emit_source()
{
    libname=$1
    extref=$2
    (
        echo "#include <stdio.h>"
        echo
        echo "void"
        echo "${libname}_func(void) {"
	echo "    printf(\"Hello world\\n\");"
	echo "}"
        echo
        echo "__attribute__((constructor))"
	echo "void"
        echo "${libname}_ctor(void) {"
        echo "    printf(\"Called %s\\n\", __FUNCTION__);"
	echo "}"
        echo
        echo "__attribute__((destructor))"
	echo "void"
        echo "${libname}_dtor(void) {"
        echo "    printf(\"Called %s\\n\", __FUNCTION__);"
	echo "}"
	if ! test -z $extref; then
            echo
	    echo "static void testref(void) {"
	    echo "    /* The following function reference can be used to show"
	    echo "       that RTLD_GLOBAL will pin the library containing that"
	    echo "       function into memory when it might otherwise have"
	    echo "       been released during dlclose() */"
	    echo
            echo "    extern void ${extref}_func(void);"
            echo "    ${extref}_func();"
            echo "}"
	fi
    ) > "lib_$libname.c"
}

#
# ====================
#

CFLAGS="-Wall -Wextra -g -Wl,--no-as-needed -Wno-unused-function"

for f in $@; do
    IFS=':' read libspec deplist <<< $f
    deplist=$(echo $deplist | tr ',' ' ')
    deplist_so=$(for d in $deplist; do echo lib_$d.so; done)
    IFS='@' read lib_suffix ref <<< $libspec
    lib_file="lib_${lib_suffix}.so"
    lib_source="lib_${lib_suffix}.c"
    emit_source $lib_suffix $ref
    cc -Wall -Wextra $CFLAGS -shared -fPIC -o $lib_file $lib_source $deplist_so
done

# To clean up: rm lib_*

#
# ====================
#

# Buid the program that will be used to load and unloaded shared libraries

cc -Wall -Wextra -Wno-unused-parameter -g -o openlibs openlibs.c -ldl
