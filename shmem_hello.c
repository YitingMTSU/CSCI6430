#include <stdio.h>
#include <stdlib.h>
#include "my_shmem.h"

int main(int argc, char* argv[]) {
    int pe, size;

    shmem_init(argc, argv);
    pe = shmem_my_pe();
    size = shmem_n_pes();

    printf("%d,size= %d\n",pe,size);
    
    shmem_finalize();
    return 0;
}
