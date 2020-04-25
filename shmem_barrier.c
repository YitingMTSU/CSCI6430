#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "my_shmem.h"

int main(int argc, char* argv[]) {
    int pe, size;
    int i = 0;
    int * sync;
    
    shmem_init(argc, argv);
    pe = shmem_my_pe();
    size = shmem_n_pes();

    sync = (int *)shmem_malloc(sizeof(int));
    //printf("malloc complete, PE:%ld\n",pe);
    *sync = 0;

    printf("%d: entering barrier\n", pe);
    if (pe == 0) {
        int release = 0;
        for (i = 1; i < size; i++) {
            int remote_var = 0;
            while (remote_var == 0) {
                shmem_int_get(&remote_var, sync, 1, i);
            }
        }
        for (i = 1; i < size; i++) {
            shmem_int_put(sync, &release, 1, i);
            sleep(1);
        }
        shmem_quiet();
    } else {
        *sync = 1;
        while (*sync != 0);
        printf("%d completed the barrier\n", pe);
    }

    shmem_finalize();
    return 0;
}
