#!/bin/sh

echo
echo "===== Compile of (just) main ('func' completely unavailable) ====="
echo

sh -c -v "cc -Wall -o prog prog.c
./prog
"

echo
echo
echo "===== Compile main with 'func' source file ====="
echo "      (dlsym() failure expected)"
echo

# In this case, dlsym() look-up fails because, by default, "func" won't
# be exported in mains's dynamic symbol table (because the linker sees
# that "func" isn't needed by any external object)

sh -c -v "cc -Wall -o prog prog.c modx.c
./prog
"

echo
echo
echo "===== Compile main with 'func' source file plus --export-dynamic ====="
echo

# In this case, dlsym() look-up succeeds because --export-dynamic causes
# all of main's symbols to be exported in main's dynamic symbol table,
# even if the linker can't see any reference that needs those symbols at
# static link time.

sh -c -v "cc -Wall -Wl,--export-dynamic -o prog prog.c modx.c
./prog
"

# =====

cc -Wall -fPIC -shared -o libx.so modx.c

# =====

echo
echo
echo "===== Linking against shared library containing 'func' ====="
echo

rpath='-Wl,--rpath,$PWD -Wl,--enable-new-dtags'
sh -c -v "cc $rpath -o prog prog.c libx.so
./prog
"

echo
echo
echo "===== Preloading shared library containing 'func' ====="
echo
sh -c -v "cc $rpath -o prog prog.c
LD_PRELOAD="libx.so" ./prog
"

echo
echo
echo "===== dlopen(RTLD_GLOBAL) library containing 'func' ====="
echo

sh -c -v "cc $rpath -o prog prog.c
./prog ./libx.so global
"

echo
echo "===== dlopen(RTLD_LOCAL) library containing 'func' ====="
echo "      (dlsym() failure expected)"
echo

# The dlsym() look-up will fail because RTLD_LOCAL means that library's
# symbols are not publicly available for subsequent dlsym() look-ups.

sh -c -v "cc $rpath -o prog prog.c
./prog ./libx.so local
"

echo
echo "===== dlopen(RTLD_LOCAL) + dlopen(RTLD_GLOBAL) library containing 'func' ====="
echo "      (failure expected on first dlsym() call)"
echo

# The first dlsym() look-up will fail because RTLD_LOCAL means that library's
# symbols are not publicly available for subsequent dlsym() look-ups.
# But the library's symbols can be promoted to "global" visibility by a
# second dlopen() with RTLD_GLOBAL ("demotion" using a subsequent
# dlopen() with RTLD_LOCAL is not possible).

sh -c -v "cc $rpath -o prog prog.c
./prog ./libx.so local global
"
