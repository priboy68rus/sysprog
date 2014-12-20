#include "shm.h"


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

//----------------------------------------------------------------

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
		// printf("1\n");
		// sleep(1);
		P(SRV)
		// printf("SRV: entering loop\n");
		while(1)
		{
			// printf("SRV: waiting in S\n");
			// printf("2\n");
			// sleep(1);
			P(S)
			// printf("SRV: reading\n");

			size = *(int *)data;
			// printf("SRV: size: %d\n", size);
			write(1, data + sizeof(int), size);
			// printf("3\n");
			// sleep(1);
			V(C)
			
			if (size < SIZE - sizeof(int))
				break;
		}
		// printf("4\n");
		// sleep(1);
		V(CLT)
	}


	return 0;
}
