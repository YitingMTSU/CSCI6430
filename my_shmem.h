#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <mpi.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/shm.h>

#ifndef OPENSHMEM_H
#define OPENSHMEM_H


#define PORT 3050

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
    keyset.keyV[i] = i + 1000;
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

//malloc the shared memory
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
  key_t barrierKey = 999;
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

void shmem_putmem(void *dest, void *source, size_t len, int pe){
  int my_pe = shmem_my_pe();
  int num = len / sizeof(source);
  void* startInd = (dest + (pe - my_pe) * num);
  char* s = (char *) startInd;
  char* t = (char *) source;
  //put the memory byte by byte
  for(int i=0;i<len;i++){
    *(s + i) = *(t + i);
  }
}


void shmem_getmem(void *dest, void *source, size_t len, int pe){
  int my_pe = shmem_my_pe();
  int num = len / sizeof(source);
  void* startInd = (source + (pe - my_pe) * num);
  char* s = (char *) startInd;
  char* t = (char *) dest;
  //get the memory byte by byte
  for(int i=0;i<len;i++){
    *(t + i) = *(s + i);
  }
}

void shmem_free(void *ptr){
  ptr = NULL;
}


void shmem_sync_all(void){
  int my_pe = shmem_my_pe();
  int pes = shmem_n_pes();
  if(my_pe == 0){
    //serve as server side
    int sockfd, new_socket;
    struct sockaddr_in server_address;
    int opt = 1;
    int addrlen = sizeof(struct sockaddr_in);
    char buffer[32];
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
      perror("socket failed");
      return;
    }
    
    // optional
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
      perror("sockopt failed");
      return;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
      perror("bind failed");
      return;
    }

    if (listen(sockfd, 10) < 0) {
      perror("listen failed:");
      return;
    }

    //only listen pes - 1
    for (int i=0;i<pes-1;i++) {
      if ((new_socket = accept(sockfd, (struct sockaddr *) &server_address, 
        (socklen_t *) &addrlen)) < 0) {
        perror("accept failed: ");
        return;
      }
      //read the new data  
      read(new_socket, buffer, 32);
      printf("Client said: %s\n", buffer);
      close(new_socket);
    }

  }else{
    //serve as client side
    int sockfd;
    struct sockaddr_in server_address;
    int addrlen = sizeof(struct sockaddr_in);
    char buffer[32];
    char address[32] = "127.0.0.1";
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return;
    }
    
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(address);
    server_address.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("Could not connect to host: ");
        return;
    }
    
    sprintf(buffer, "Hello, this is pe %d", my_pe);
    send(sockfd, buffer, strlen(buffer), 0);
    close(sockfd);

  }
}

void shmem_put_nbi(void *dest, void *source, size_t nelems, int pe){
  int my_pe = shmem_my_pe();
  int num = nelems / sizeof(source);
  void* startInd = (dest + (pe - my_pe) * num);
  char* s = (char *) startInd;
  char* t = (char *) source;
  //put the memory byte by byte
  for(int i=0;i<nelems;i++){
    *(s + i) = *(t + i);
  }
}

void shmem_get_nbi(void *dest, void *source, size_t nelems, int pe){
  int my_pe = shmem_my_pe();
  int num = nelems / sizeof(source);
  void* startInd = (source + (pe - my_pe) * num);
  char* s = (char *) startInd;
  char* t = (char *) dest;
  //get the memory byte by byte
  for(int i=0;i<nelems;i++){
    *(t + i) = *(s + i);
  }
}

#endif
