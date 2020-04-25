#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <mpi.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include <sys/shm.h>

#ifndef OPENSHMEM_H
#define OPENSHMEM_H

int intKeyVal = 12345;
int shmInt_id;
int* sharedInt;

//initilize the openshmem
void shmem_init(int argc, char* argv[]){
  //MPI_Init();
  MPI_Init(&argc, &argv);
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
  shmdt(sharedInt);
  shmctl(shmInt_id, IPC_RMID, NULL);
  MPI_Finalize();
}

//malloc the shared memory for integer type
int *shmem_malloc(size_t size){
  key_t intKey = intKeyVal;

  int pes = shmem_n_pes();

  //int shmInt_id;
  
  shmInt_id = shmget(intKey, pes*size, 0666|IPC_CREAT);
  if(shmInt_id == -1){
    perror("Shared memory failed:");
    return (int*)NULL;
  }

  //malloc the same size memory locally
  //int* sharedInt;
  //shmdt(shared);
  sharedInt = (int *)malloc(pes*size);
  sharedInt = shmat(shmInt_id, NULL, 0);

  //shmdt(shared);
  int pe = shmem_my_pe();
  return &sharedInt[pe];
}

void shmem_int_get(int* target, int* source, int len, int pe){
  int my_pe = shmem_my_pe();
  if(my_pe == pe){
    *target = *source;
  }else{
    int dis = pe - my_pe;
    *target = *(source + dis);
  }
  
}

void shmem_int_put(int* source, int* target, int len, int pe){
  int my_pe = shmem_my_pe();
  if(my_pe == pe){
    *source = *target;
  }else{
    int dis = pe - my_pe;
    *(source + dis) = *target;
  }
}

void shmem_quiet(void){

}

#endif
