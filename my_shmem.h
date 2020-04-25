#ifndef MPI
#define MPI
#include<mpi.h>

#endif

#include<stdlib.h>
#include<stdio.h>

#ifndef OPENSHMEM_H
#define OPENSHMEM_H

//initilize the openshmem
void shmem_init(void){
  //MPI_Init();
  MPI_Init(int argc, char* argv[]);
}

//return the number of PEs 
int shmem_n_pes(){
  int size;
  MPI_Comm_size(MPI_COMM_WORLD,&size);
  return size;
}

//return the current index of PE
int shmem_my_pe(){
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  return rank;
}

//finalize the openshmem
void shmem_finalize(){
  MPI_Finalize();
}


#endif
