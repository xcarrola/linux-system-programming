TARGETS = prog libmod.so

all: ${TARGETS}

prog: prog.o
	cc -Wl,--export-dynamic -Wl,-rpath,${PWD} -Wl,--enable-new-dtags \
	    	-o prog prog.c -ldl

libmod.so: mod.c
	cc -c -fPIC -o mod.o mod.c
	cc -shared -o libmod.so mod.o

clean:
	${RM} ${TARGETS} *.o
