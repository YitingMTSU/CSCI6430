#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/time.h>
#include<shmem.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<fcntl.h>

#define SHM_SIZE 1024
#define NAME_SIZE 255

int main(){
  //my_pe: the index of current pe;
  //n_pes: the number of total pes;
  int my_pe, n_pes;

  
  //shmem initialize
  shmem_init();

  //get the pe index and total pes
  my_pe = shmem_my_pe();
  n_pes = shmem_n_pes();

  //shared memory block
  char** shared;
  shared = (char**)malloc(n_pes*sizeof(char*));

  //create name vector to save all the combination
  //the format of name will always be num1..num2(num1<num2)
  char** name;
  
  //malloc the memory for name
  name = (char**)malloc(n_pes*sizeof(char*));
  for(int i=0;i<n_pes;i++){
    name[i] = (char *)malloc(NAME_SIZE*sizeof(char));
  }

  for(int i=0;i<my_pe;i++){
    sprintf(name[i],"%d..%d",i,my_pe);
  }
  for(int i=my_pe+1;i<n_pes;i++){
    sprintf(name[i],"%d..%d",my_pe,i);
  }

  
  
  //create/allocate shared object
  for(int i=my_pe+1;i<n_pes;i++){
    key_t mykey = ftok(name[i],0);
    if(my_key == -1){
      perror("ftok failed: ");
      return -1;
    }
    //create/allocate shared object for each communication
    int shm_id = shmget(mykey,SHM_SIZE,0666 | IPC_CREAT);
    if(shm_id == -1){
      perror("Shared memory failed: ");
      return -1;
    }
    //map shared memory
    shared[i] = (char *) shmat(shm_id,NULL,0);
    if(shared[i] == (char *)-1){
      perror("shnat: ");
      return -1;
    }
    memset(shared[i],0,SHM_SIZE);
  }
    
  shmem_barrier_all();
  

  if(my_pe == 0){ //the main process
    printf("This is main process my_pe:%d\n",my_pe);
    //char* tmp = "Here is 1's information.";
    //sprintf(shared+1, "%s", "what a day!");
    for(int i=0;i<n_pes;i++){
      if(i != my_pe)
      printf("%d ,name[%d] : %s\n",my_pe,i,name[i]);
    }
    int count = 0;
    while(1){
      for(int i=1;i<n_pes;i++){
	if(shared[i][0] == 1){
	  printf("Receive message from PE: %d, Message:%s\n",i,shared[i][0]+1);
	  share[i][0] = 0;
	  count++;
	}
      }
      if(count == n_pes - 1) break;
    }
    //shared[0] = 1;
    shmdt(shared);
    
  }else{ //other process
    printf("This is child process my_pe:%d\n",my_pe);
    //char* tmp = "Here is 2's information.";
    //sleep(1);
    //sprintf(shared+1, "%s", tmp);
    for(int i=0;i<n_pes;i++){
      if(i != my_pe)
      printf("%d ,name[%d] : %s\n",my_pe,i,name[i]);
    }
    //send message to 0 PE by shared memory
    {
      key_t mykey = ftok(name[0],0);
      if(mykey == -1){
	perror("ftok falied: ");
	return -1;
      }
      int shm_id = shmget(mykey,SHM_SIZE,0666);
      if(shared == -1){
	perror("Shared Memory Failed: ");
	return -1;
      }
      char* sharedPE = (char*) shmat(shm_id,NULL,0);
      if(sharedPE == (char*)-1){
	perror("shmat: ");
	return -1;
      }
      sprintf(sharedPE[0]+1,"This is the message from PE:%d",my_pe);
      sharedPE[0] = 1;
      shmdt(sharedPE);
    }
    //printf("%d,shared : %s\n",my_pe,shared);
    //shmdt(shared);
  }
  //shmdt(shared);
  shmem_barrier_all();

  free(shared);
  free(name);
      
  //shmem finalize
  shmem_finalize();

  //munmap(shared, SHM_SIZE);
  //close(fd);

  return 0;  
}
