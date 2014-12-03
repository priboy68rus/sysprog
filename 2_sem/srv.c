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
#define CLT 0
#define SRV 1
#define MTX 2


int main(int argc, char *argv[], char *envp[])
{
	char * data; //Shared memory
	int shmid; //Shm descriptor
	int new = 1;
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

	if ((shmid = shmget(key, 4096 * sizeof(char), 0666|IPC_CREAT|IPC_EXCL)) < 0)
	{
		if (errno != EEXIST)
		{
			printf("Cant create shared memory\n");
			exit(-1);
		} 
		else
		{
			if ((shmid = shmget(key, 4096 * sizeof(char), 0)) < 0)
			{
				perror(argv[0]);
				exit(errno);
			}
			new = 0;
		}
		// perror(argv[0]);
		// exit(errno);
	}

	if ((data = (char *)shmat(shmid, NULL, 0)) == (char *)(-1))
	{
		perror(argv[0]);
		exit(errno);
	}

	if (new)
	{
		for (i = 0; i < 4096; i++)
		{
			data[i] = 0;
		}
	}

	int semid;
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

	if ((semid = semget(key, 3, 0666 | IPC_CREAT)) < 0)
	{
		printf("Cant get semid\n");
		exit(errno);
	}


	while (1)
	{
		printf("Waiting in srv\n");
		pbuf.sem_num = SRV;
		if (semop(semid, &pbuf, 1) < 0)
		{
			printf("Cant wait\n");
			exit(-1);
		}

		printf("Waiting in mutex\n");
		pbuf.sem_num = MTX;
		if (semop(semid, &pbuf, 1) < 0)
		{
			printf("Cant wait\n");
			exit(-1);
		}

		// Read and print

		vbuf.sem_num = MTX;
		if (semop(semid, &vbuf, 1) < 0)
		{
			printf("Cant wait\n");
			exit(-1);
		}

		vbuf.sem_num = CLT;
		if (semop(semid, &vbuf, 1) < 0)
		{
			printf("Cant wait\n");
			exit(-1);
		}

	}


	return 0;
}
