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

//max shared block number
#define MAX_BLOCK  100
//manage the shared memory
struct keySet{
  int keyV[MAX_BLOCK];
  int shm_id[MAX_BLOCK];
  void* shared[MAX_BLOCK];
  int block_num;
};

struct keySet keyset;

//initilize the openshmem
void shmem_init(int argc, char* argv[]){
  //MPI_Init();
  keyset.block_num = 0;
  for(int i=0;i<MAX_BLOCK;i++){
    keyset.keyV[i] = i^2;
  }

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
  for(int i=0;i<keyset.block_num;i++){
    void* shared = keyset.shared[i];
    int shm_id = keyset.shm_id[i];
    shmdt(shared);
    shmctl(shm_id, IPC_RMID, NULL);
  }
  MPI_Finalize();
}

//malloc the shared memory for integer type
void* shmem_malloc(size_t size){
  int ind = keyset.block_num;
  keyset.block_num++;

  key_t intKey = keyset.keyV[ind];

  int pes = shmem_n_pes();

  //int shmInt_id;
  
  keyset.shm_id[ind] = shmget(intKey, pes*size, 0666|IPC_CREAT);
  int id = keyset.shm_id[ind];
  if(id == -1){
    perror("Shared memory failed:");
    return NULL;
  }

  //malloc the same size memory locally
  keyset.shared[ind] = malloc(pes*size);
  keyset.shared[ind] = shmat(id, NULL, 0);

  void* p = keyset.shared[ind];
  //shmdt(shared);
  int pe = shmem_my_pe();
  return (p + pe*size);
}

void shmem_int_get(int* target, int* source, int len, int pe){
  int my_pe = shmem_my_pe();
  if(my_pe == pe){
    if(len == 1){
      *target = *source;
    }else{
      for(int i=0;i<len;i++){
        *(target + i) = *(source + i);
      }
    }
  }else{
    int dis = pe - my_pe;
    int startInd = dis*len;
    if(len == 1){ 
      *target = *(source + dis);
    }else{
      for(int i=0;i<len;i++){
        *(target +i) = *(source + i);
      }
    }
  }
  
}

void shmem_int_put(int* source, int* target, int len, int pe){
  int my_pe = shmem_my_pe();
  if(my_pe == pe){
    if(len == 1){
      *source = *target;
    }else{
      for(int i=0;i<len;i++){
        *(source + i) = *(target + i);
      }
    }
  }else{
    int dis = pe - my_pe;
    int startInd = dis*len;
    if(len == 1){
      *(source + dis) = *target;
    }else{
      for(int i=0;i<len;i++){
        *(source + i) = *(target + i);
      }
    }
  }
}

void shmem_quiet(void){
  //no idea what to do here
}

void shmem_barrier_all(void){
  int my_pe = shmem_my_pe();
  key_t barrierKey = rand() % 10000;
  int pes = shmem_n_pes();
  int shmBarrierId = shmget(barrierKey, pes*sizeof(int),0666|IPC_CREAT);
  int* sharedBarrier = (int *) malloc(pes*sizeof(int));
  sharedBarrier = shmat(shmBarrierId, NULL, 0);
  int* cur = &sharedBarrier[my_pe];
  *cur = 0;
  if(my_pe == 0){
    int release = 0;
    for(int i=1;i<pes;i++){
      int remote_var = 0;
      while(remote_var == 0){
        shmem_int_get(&remote_var, cur, 1, i);
      }
    }
    for(int i=1;i<pes;i++){
      shmem_int_put(cur, &release, 1, i);
      sleep(0.5);
    }
    shmem_quiet();
    shmdt(sharedBarrier);
  }else{
    *cur = 1;
    while(*cur != 0);
    //printf("%d complete.\n",my_pe);
    shmdt(sharedBarrier);
  }
  shmctl(shmBarrierId, IPC_RMID, NULL);  
}

#endif
