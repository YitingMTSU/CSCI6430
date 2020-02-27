OSHCC = oshcc
CFLAGS = 
all: openShmem

openShmem: openShmem.c
	${OSHCC} ${CFLAGS} -o openShmem openShmem.c

clean:
	rm openShmem
	rm *~
