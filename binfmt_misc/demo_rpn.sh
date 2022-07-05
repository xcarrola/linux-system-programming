#!/bin/sh
#
# See the kernel source file Documentation/admin-guide/binfmt-misc.rst
#
# Shell script to demonstate registration of an interpreter (an RPN
# calculator) using the binfmt_misc mechanism.

if ! test -e  /proc/sys/fs/binfmt_misc/register; then
    echo "binfmt_misc is not mounted; mounting..."
    sudo mount binfmt_misc -t binfmt_misc /proc/sys/fs/binfmt_misc
fi

# Create interpreter

cc -o rpn rpn.c -lm

# Enable execute permission on file that will be interpreted

chmod +x rscript.rpn

binfmt_file="/proc/sys/fs/binfmt_misc/rpn"

# Create binfmt_misc entry for interpreter

if test -e $binfmt_file; then
    echo "Removing preexisting $binfmt_file"
    sudo sh -c "echo -1 > $binfmt_file"
fi

interp="$PWD/rpn"
echo "Registering $interp in /proc/sys/fs/binfmt_misc/register"

if test $# -eq 0; then
    echo "Registering interpreter by extension (.rpn)"
    sudo sh -c "echo ':rpn:E::rpn::$interp:' > /proc/sys/fs/binfmt_misc/register"
else
    echo "Registering interpreter by magic (\"#rpn\")"
    sudo sh -c "echo ':rpn:M::#rpn::'$interp':' > /proc/sys/fs/binfmt_misc/register"
fi

echo
echo "Contents of $binfmt_file:"
cat $binfmt_file
echo

echo "=========="
echo

echo "About to execute ./rscript.rpn"
./rscript.rpn
