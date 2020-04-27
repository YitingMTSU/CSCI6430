Project:
Class: CSCI 6430 Selected Topics in Parallel
Name: Yiting Wang
Date: 2020 Spring

Project Description:
Implemnt part of the OpenSHMEM libarary
Different PEs communicate with shared memory
Differnt process communicate with active message
Use MPI package


Usage:
make //make the *.exe files for all the tests
mpirun -np $(n) ./*.exe //run the exe file. Here n means the number of PEs

function implement:
shmem_init()
shmem_my_pe()
shmem_n_pes()
shmem_finalize()

shmem_put()
shmem_get()
shmem_put_nbi()
shmem_get_nbi()
shmem_putmem()
shmem_getmem()

shmem_barrier_all()
shmem_sync_all()

shmem_malloc()

shmem_quiet()

