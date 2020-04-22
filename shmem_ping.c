#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <shmem.h>

double timestamp()
{
    double retval;
    struct timeval tv;
    if (gettimeofday(&tv, NULL)) {
        perror("gettimeofday");
        abort();
    }
    retval = ((double)tv.tv_sec) * 1000000 + tv.tv_usec;
    return retval;
}

int main(void)
{
    int my_pe, n_pes;
    int * ping;
    int i = 0, j = 0;
    double start, end;
    
    shmem_init();
    my_pe = shmem_my_pe();
    n_pes = shmem_n_pes();

    for (int i = 1; i <= 1024 * 1024; i = i << 1) {
        ping = (int *) shmem_malloc(sizeof(int) * i);
        memset(ping, 0, sizeof(int) * i);
        shmem_barrier_all();
        if (my_pe == 0) {
            ping[i - 1] = 1;
            start = timestamp();
            shmem_int_put(ping, ping, i, 1);
            shmem_quiet();
            while (ping[i - 1] != 2);
            end = timestamp();
            printf("[%d] ping-pong with %d size in %0.4g us\n", 0, sizeof(int) * i, end - start);
//            shmem_wait_until(ping[i - 1], SHMEM_CMP_EQ, 2);
        } else {
//            shmem_wait_until(ping[i - 1], SHMEM_CMP_EQ, 1);
            while (ping[i - 1] != 1); 
            ping[i - 1] = 2;
            shmem_int_put(ping, ping, i, 0);
            shmem_quiet();
        }
        shmem_barrier_all();
        shmem_free(ping);
    }
            
    shmem_finalize();
    return 0;
}
