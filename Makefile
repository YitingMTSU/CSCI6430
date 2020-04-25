MPICC = mpicc
CFLAGS = -I

DEPS = my_shmem.h

all: hello.exe

hello.exe: shmem_hello.o
	$(MPICC) $^ -o $@

shmem_hello.o: shmem_hello.c
	$(MPICC) -c $^ -o $@




clean:
	rm -f *.o *.exe *.out