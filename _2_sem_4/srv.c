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
		printf("SRV: P: Cant wait\n");		\
		exit(-1);							\
	}										\

#define V(sem)								\
	vbuf.sem_num = sem;						\
	if (semop(semid, &vbuf, 1) < 0)			\
	{										\
		printf("SRV: V: Cant wait\n");		\
		exit(-1);							\
	}										\


int main(int argc, char *argv[], char *envp[])
{
	char * data; //Shared memory
	int shmid; //Shm descriptor
	int new = 1, size = 0;
	char pathname[] = "srv.c";
	key_t key;

	int i = 0;

	// Creating and opening shared memory

	if ((key = ftok(pathname, 0)) < 0)
	{
		printf("Can't generate key\n");
		perror(argv[0]);
		exit(errno);
	}

	if ((shmid = shmget(key, SIZE * sizeof(char), 0666|IPC_CREAT|IPC_EXCL)) < 0)
	{
		if (errno != EEXIST)
		{
			printf("Cant create shared memory\n");
			exit(-1);
		} 
		else
		{
			if ((shmid = shmget(key, SIZE * sizeof(char), 0)) < 0)
			{
				printf("Cant find shared memory\n");
				perror(argv[0]);
				exit(errno);
			}
			new = 0;
		}
	}

	if ((data = (char *)shmat(shmid, NULL, 0)) == (char *)(-1))
	{
		perror(argv[0]);
		exit(errno);
	}

	if (new)
	{
		for (i = 0; i < SIZE; i++)
		{
			data[i] = 0;
		}
	}

	int semid;
	struct sembuf vbuf, pbuf;
	char pathname_sem[] = "clt.c";

	vbuf.sem_op = 1;
	vbuf.sem_flg = SEM_UNDO;

	pbuf.sem_op = -1;
	pbuf.sem_flg = SEM_UNDO;

	if((key = ftok(pathname_sem,0)) < 0){
		printf("Can\'t generate key\n");
		exit(-1);
	}

	if ((semid = semget(key, 4, 0666 | IPC_CREAT)) < 0)
	{
		printf("Cant get semid\n");
		exit(errno);
	}


	while (1)
	{
		// printf("SRV: waiting in SRV\n");
		P(SRV)
		// printf("SRV: entering loop\n");
		while(1)
		{
			// printf("SRV: waiting in S\n");
			P(S)
			// printf("SRV: reading\n");

			size = *(int *)data;
			// printf("SRV: size: %d\n", size);
			write(1, data + sizeof(int), size);
			V(C)
			
			if (size < SIZE - sizeof(int))
				break;
		}

		V(CLT)
	}


	return 0;
}
