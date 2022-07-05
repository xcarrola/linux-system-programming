#!/bin/sh

show_libx_dynsyms()
{
echo "(Relevant) dynamic symbols exposed by libx.so:"
echo
sh -c "
echo -n '    $ '
set -v
objdump --dynamic-syms libx.so | grep xxx | cut -c49- | sed 's/^/    /'
"
}

show_required_relocs()
{
echo "Relocations required by $1:"
echo
sh -c "
echo -n '    $ '
set -v
objdump --dynamic-reloc $1 | sed 's/^/    /'
"
}

echo
echo "========== Building library without symbol versioning =========="
echo

# Build non-symbol-versioned library

cc -Wall -fPIC -shared -o libx.so libx_v1.c

show_libx_dynsyms

# Make a copy of the unversioned library, in case user wants to
# inspect it later with readelf/objdump

cp libx.so libx_non_symver.so

echo
echo "========== Building binary 'p1' against library =========="
echo

# Build binary that links against non-symbol-versioned library

cc -Wall -o p1 -Wl,-rpath,'$ORIGIN' prog.c libx.so

echo "========== Building symbol-versioned library =========="
echo

# Build symbol-versioned library

cc -Wall -fPIC -shared -Wl,--version-script,libx.map -o libx.so libx_v2.c

show_libx_dynsyms

echo
echo "Versions defined in libx.so:"
echo
sh -c "
echo -n '    $ '
set -v
readelf --version-info libx.so | sed -n '/.gnu.version_d/,/^$/p' |
                 sed 's/^/    /'
"

echo "========== Building binary 'p2' against symbol-versioned " \
	 "library =========="
echo

# Build binary that links against symbol-versioned library

cc -Wall -o p2 -Wl,-rpath,'$ORIGIN' prog.c libx.so

show_required_relocs p1

show_required_relocs p2


echo "========== Executing binaries =========="
echo

set -v
./p1	        # Was linked against non-SV library

./p2
