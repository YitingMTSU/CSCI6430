#include <stdio.h>
#include <stdlib.h>
#include <shmem.h>
#include <sys/time.h>

#define ITER    10000
#define SKIP    1000

static inline double TIME(void) {
    double wtime = 0;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    wtime = tv.tv_sec;
    wtime += (double) tv.tv_usec / 1000000.0;
    return wtime;
}

int main(int argc, char ** argv) {
    int pe, size;
    long * sbuf, * rbuf;
    size_t size_b;
    int i = 0;
    double t_start, t_end;

    if (argc < 2) {
        size_b = 1;
    } else {
        size_b = atoi(argv[1]);
    }
    
    shmem_init();
    pe = shmem_my_pe();
    size = shmem_n_pes();
    sbuf = (long *) shmem_malloc(size_b);
    rbuf = (long *) shmem_malloc(size_b);

    for (i = 0; i < size_b; i++) {
        sbuf[i] = 'a';
        rbuf[i] = 'b';
    }
    shmem_barrier_all();

    if (pe == 0) {
        for (; i < ITER + SKIP; i++) {
            if (i == SKIP) {
                t_start = TIME();
            }

            shmem_putmem(rbuf, sbuf, size_b, 1);
            shmem_quiet();
        }
        t_end = TIME();
    }
    shmem_barrier_all();       

    if (pe == 0) {
        double latency = (1.0 * t_end - t_start) / ITER;
        printf("time taken: %g - %g = %g\n", t_end, t_start, t_end - t_start);
        printf("put latency: %g s\n", latency);
        fflush(stdout);
    }

    shmem_finalize();
    return 0;
}
