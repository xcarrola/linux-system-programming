include ../Makefile.inc

GEN_EXE = svshm_attach svshm_create svshm_mon svshm_rm \
	svshm_xfr_reader svshm_xfr_writer 

LINUX_EXE = svshm_info svshm_lock svshm_unlock

COURSE_FILES_EXE = svshm_psem_seqnum_get svshm_psem_seqnum_init

EXERCISE_FILES_EXE = # ex.svshm_psem_xfr_reader ex.svshm_psem_xfr_writer

EXE = ${GEN_EXE} ${LINUX_EXE} ${COURSE_FILES_EXE} ${EXERCISE_FILES_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

ex.svshm_psem_xfr_writer ex.svshm_psem_xfr_reader: svshm_psem_xfr.h

svshm_psem_xfr_test:
	dd if=/dev/urandom of=/tmp/infile count=100000
	./ex.svshm_psem_xfr_writer < /tmp/infile &
	./ex.svshm_psem_xfr_reader > /tmp/outfile
	@echo
	@if diff -q /tmp/infile /tmp/outfile; then echo "Test passed" ; else echo "Test FAILED"; fi

clean : 
	${RM} ${EXE} *.o

svshm_xfr_reader.o svshm_xfr_writer.o: svshm_xfr.h

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
