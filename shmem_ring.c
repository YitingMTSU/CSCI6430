#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <shmem.h>

int main(void) {
    int pe, size;
    int * token;
    int target;
    int val = 1;

    shmem_init();

    token = (int *)shmem_malloc(sizeof(int));
    pe = shmem_my_pe();
    size = shmem_n_pes();
    *token = 0;
    target = (pe + 1) % size;
    printf("[debug] pe: %d target: %d\n", pe, target);
    fflush(stdout);

    shmem_barrier_all();

    // begin token passing
    if (pe == 0) {
        shmem_int_put(token, &val, 1, target); 
        while (*token == 0);
        printf("[%d] token received\n", pe);
    } else {
        while (*token == 0);
        printf("[%d] token received\n", pe);
        sleep(1);
        shmem_int_put(token, &val, 1, target);
    }

    shmem_barrier_all();
    shmem_finalize();
    return 0;
}
        
