MPICC = mpicc
CFLAGS = -I

DEPS = my_shmem.h

all: hello.exe barrier.exe ring.exe

hello.exe: shmem_hello.o
	$(MPICC) $^ -o $@

shmem_hello.o: shmem_hello.c
	$(MPICC) -c $^ -o $@

barrier.exe: shmem_barrier.o
	$(MPICC) $^ -o $@

shmem_barrier.o: shmem_barrier.c
	$(MPICC) -c $^ -o $@

ring.exe: shmem_ring.o
	$(MPICC) $^ -o $@

shmem_ring.o: shmem_ring.c
	$(MPICC) -c $^ -o $@


clean:
	rm -f *.o *.exe *.out