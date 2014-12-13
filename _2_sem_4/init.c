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

#define SIZE 1024

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


int main(int argc, char *argv[], char *envp[])
{
	int semid;
	key_t key;
	struct sembuf vbuf, pbuf;
	char pathname_sem[] = "clt.c";

	vbuf.sem_op = 1;
	vbuf.sem_flg = 0;

	pbuf.sem_op = -1;
	pbuf.sem_flg = 0;

	if((key = ftok(pathname_sem,0)) < 0){
		printf("Can\'t generate key\n");
		exit(-1);
	}

	if ((semid = semget(key, 4, 0666 | IPC_CREAT)) < 0)
	{
		printf("Cant get semid\n");
		exit(errno);
	}

	V(CLT)
	V(C)

	return 0;
}
