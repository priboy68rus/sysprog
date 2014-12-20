#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/sem.h>

#define SIZE 102

#define CLT 0
#define SRV 1
#define C 2
#define S 3

#define P(sem)								\
	pbuf.sem_num = sem;						\
	if (semop(semid, &pbuf, 1) < 0)			\
	{										\
		printf("Cant wait\n");				\
		exit(-1);							\
	}										\

#define V(sem)								\
	vbuf.sem_num = sem;						\
	if (semop(semid, &vbuf, 1) < 0)			\
	{										\
		printf("Cant wait\n");				\
		exit(-1);							\
	}										\
