#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/time.h>
#include<shmem.h>

int main(){
  //my_pe: the index of current pe;
  //n_pes: the number of total pes;
  int my_pe, n_pes;

  //shmem initialize
  shmem_init();

  //get the pe index and total pes
  my_pe = shmem_my_pe();
  n_pes = shmem_n_pes();

  if(my_pe == 0){ //the main process
    printf("This is main process my_pe:%d\n",my_pe);
  }else{ //other process
    printf("This is child process my_pe:%d\n",my_pe);
  }
  
  
  //shmem finalize
  shmem_finalize();

  return 0;  
}
