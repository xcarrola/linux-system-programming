Given that:
* z3 depends on z2 which depends on z1

First, we build the libraries (and the test program, openlibs.c):

    $ rm -f lib*; sh Build_lib.sh z1 z2:z1 z3:z2

If z3 gets loaded it pulls in the other two libraries, and the
constructors are called, with the constructor of the deepest
dependency being called first; the destructors are called in reverse
order when the program terminates:

    $ LD_LIBRARY_PATH=. ./openlibs lib_z3.so
    [0] Opening lib_z3.so
    Called z1_ctor
    Called z2_ctor
    Called z3_ctor

    Program about to exit
    Called z3_dtor
    Called z2_dtor
    Called z1_dtor

The same again, but this time explicitly dlclose() handle 0 (lib_z3.so)
to show that the libraries are unloaded (and their destructors called)
before program exit:

    $ LD_LIBRARY_PATH=. ./openlibs lib_z3.so  -0
    [0] Opening lib_z3.so
    Called z1_ctor
    Called z2_ctor
    Called z3_ctor

    Closing handle 0 (lib_z3.so)
    Called z3_dtor
    Called z2_dtor
    Called z1_dtor

    Program about to exit

Now explicitly open z3 (which implicitly opens z2 and z1). At that
point all three constructors are called. Then, explicitly open z1 (at
which point the z1 constructor is not called, because z1 was already
loaded).  Then, explicitly close handle 0 (z3); at this point the
destructors for z3 and z2 are called (and the libraries are unloaded),
but the destructor for z1 is not (because it has been opened twice).
Finally, close handle 1 (z1), at which point the library is unloaded
and the z1 destructor is called).

    $ LD_LIBRARY_PATH=. ./openlibs lib_z3.so lib_z1.so -0 -1
    [0] Opening lib_z3.so
    Called z1_ctor
    Called z2_ctor
    Called z3_ctor

    [1] Opening lib_z1.so

    Closing handle 0 (lib_z3.so)
    Called z3_dtor
    Called z2_dtor

    Closing handle 1 (lib_z1.so)
    Called z1_dtor

    Program about to exit

=====

Given that:
* x1, y1, and rx do not depend on one another
* rx includes a reference to a function (x1_func() in x1)

First, we build the libraries (and the test program, openlibs.c):

    $ rm -f lib*; sh Build_lib.sh x1 y1 rx@x1

Open x1 and y1, then close handle 0 (x1) and then handle 1 (y1):

    $ LD_LIBRARY_PATH=. ./openlibs lib_x1.so lib_y1.so -0 -1
    [0] Opening lib_x1.so
    Called x1_ctor

    [1] Opening lib_y1.so
    Called y1_ctor

    Closing handle 0 (lib_x1.so)
    Called x1_dtor

    Closing handle 1 (lib_y1.so)
    Called y1_dtor

    Program about to exit

From the above, we see that the x1 destructor was called (and the
library unloaded) immediately when handle 0 was closed. This
contrasts with the following.

Open x1 and rx, then close handle 0 (x1) and then handle 1 (rx):

    $ LD_LIBRARY_PATH=. ./openlibs lib_x1.so lib_rx.so -0 -1
    [0] Opening lib_x1.so
    Called x1_ctor

    [1] Opening lib_rx.so
    Called rx_ctor

    Closing handle 0 (lib_x1.so)

    Closing handle 1 (lib_rx.so)
    Called rx_dtor
    Called x1_dtor

    Program about to exit

In this case, the x1 destructor was not called when handle 0 (x1) was
closed, because x1 had served to satisfy a relocation (for the
reference to x1_func()) for rx. Therefore, the x1 destructor was
called (and x1 was unloaded)  only when rx (handle 1) was closed.
