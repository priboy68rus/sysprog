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
	if (argc < 2)
	{
		printf("Wrong number of arguments\nExpected 2\n");
		exit(-1);
	}

	char * data; //Shared memory
	int shmid; //Shm descriptor
	int new = 1;
	char pathname[] = "srv.c";
	key_t key;
	
	int i = 0;
	int fd = -1;
	int size = 0;
	char * buf;

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
		for (i = 0; i < 4096; i++)
		{
			data[i] = 0;
		}
	}

	// Opening file
	
	if ((fd = open(argv[1], O_RDONLY, 0666)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}

	/*
	// Writing to stdout
	
	buf = (char *)malloc(4096 * sizeof(char));
	
	while (1)
	{
		if ((size = read(fd, buf, 4096)) < 0)
		{
			perror(argv[0]);
			exit(errno);
		}

		if ((write(1, buf, size)) < 0)
		{
			perror(argv[0]);
			exit(errno);
		}

		if (size < 4096)
			break;
	}
	*/



	// Writing to shm
	/*
	while (1)
	{
		if ((size = read(fd, buf, 4096)) < 0)
		{
			perror(argv[0]);
			exit(errno);
		}

		for (i = 0; i < size; i++)
		{
			data[i] = buf[i];
		}

		if (size < 4096)
			break;
	}
	*/
	
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

	if ((semid = semget(key, 3, 0666 | IPC_CREAT)) < 0)
	{
		printf("Cant get semid\n");
		exit(errno);
	}	

	// 0 - clt, 1 - srv, 2 - mutex

	printf("Waiting in clt\n");
	pbuf.sem_num = CLT;
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

	// Send

	buf = (char *)malloc(4096 * sizeof(char));

	if ((size = read(fd, buf, 4096)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}

	for (i = 0; i < 4096; i++)
	{
		if (i < size)
			data[i] = buf[i];
		else
			data[i] = 0;
	}

	for (i = 0; i < 26; i++)
		data[i] = 'A' + i;


	// sleep(1);

	vbuf.sem_num = MTX;
	if (semop(semid, &vbuf, 1) < 0)
	{
		printf("Cant wait\n");
		exit(-1);
	}

	vbuf.sem_num = SRV;
	if (semop(semid, &vbuf, 1) < 0)
	{
		printf("Cant wait\n");
		exit(-1);
	}	



	return 0;
}
