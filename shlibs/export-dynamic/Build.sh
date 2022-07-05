#!/bin/sh
cc -fPIC -g -shared -o libdynload.so libdynload.c
cc -fPIC -g -shared -o libabc.so libabc.c

echo "Building main without --export-dynamic"
echo
cc $FLAGS -ldl prog.c -o prog libabc.so

LD_LIBRARY_PATH=. ./prog libdynload.so main_f1

echo

# The dlsym() call performed in the following program execution is
# expected to fail
LD_LIBRARY_PATH=. ./prog libdynload.so main_f2

echo
echo
echo "Building main with --export-dynamic"
echo
cc -Wl,--export-dynamic -ldl prog.c -o prog.expdyn libabc.so

LD_LIBRARY_PATH=. ./prog.expdyn libdynload.so main_f1

echo

LD_LIBRARY_PATH=. ./prog.expdyn libdynload.so main_f2
