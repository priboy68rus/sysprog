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

	// Opening file
	
	if ((fd = open(argv[1], O_RDONLY, 0666)) < 0)
	{
		perror(argv[0]);
		exit(errno);
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

	printf("CLT: waiting in CLT\n");
	P(CLT)

	// Send
	buf = (char *)malloc(SIZE * sizeof(char));

	while (1)
	{
		printf("CLT: waiting in C\n");
		P(C)
		if ((size = read(fd, buf, SIZE - sizeof(int))) < 0)
		{
			perror(argv[0]);
			exit(errno);
		}

		*(int *)data = size;
		for (i = 0; i < SIZE - sizeof(int); i++)
		{
			if (i < size)
				data[i+sizeof(int)] = buf[i];
			else
				data[i+sizeof(int)] = 0;
		}
		printf("CLT: size: %d\n", size);

		V(S)
		if (size < SIZE - sizeof(int))
			break;
	}

	V(SRV)

	return 0;
}
