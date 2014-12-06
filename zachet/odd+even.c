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
#include <sys/msg.h>

struct mymsgbuf
{
	long mtype;
	char mtext[0];
};


struct mymsgbuf * mybuf;

char pathname[] = "msg+thread.c";
int k = 10;

void * my_thread(void *a)
{
	int i = *(int *)a;
	i++;
	key_t key;
	int msqid;

	if ((key = ftok(pathname, 0)) < 0)
	{
		exit(errno);
	}

	if ((msqid = msgget(key, 0666 | IPC_CREAT)) < 0)
	{
		exit(errno);
	}
	// printf("msqid: %d\n", msqid);


	if ((msgrcv(msqid, (struct msgbuf *) mybuf, 2, i, 0)) < 0)
	{
		exit(errno);
	}

	printf("RECEIVED\n");

	printf("NUM: %d THID: %d\n", i, (int)pthread_self());


	// if (i % 2 == 0)
	// {
	// 	if (i == 2)
	// 		mybuf->mtype = 255;
	// 	else
	// 		mybuf->mtype = i-2;
	// }
	// else
	// {
	// 	if (i == 9)
	// 		mybuf->mtype = 10;
	// 	else
	// 		mybuf->mtype = i+2;
	// }

	if (k % 2 == 0)
	{
		if (i % 2 == 0)
		{
			if (i == 2)
				mybuf->mtype = 255;
			else
				mybuf->mtype = i-2;
		}
		else
		{
			if (i == k-1)
				mybuf->mtype = k;
			else
				mybuf->mtype = i+2;
		}
	}
	else
	{
		if (i % 2 == 0)
		{
			if (i == 2)
				mybuf->mtype = 255;
			else
				mybuf->mtype = i-2;
		}
		else
		{
			if (i == k)
				mybuf->mtype = k-1;
			else
				mybuf->mtype = i+2;
		}
	}

	printf("mtype: %ld\n", mybuf->mtype);

	if ((msgsnd(msqid, (struct msgbuf *) mybuf, 2, 0)) < 0)
	{
		exit(errno);
	}

	printf("SENT\n");
	return NULL;
}


int main(int argc, char *argv[], char *envp[])
{
	key_t key;
	int msqid;
	pthread_t thid[k];
	int i = 0;
	int result = 0;
	int num[k];

	if (argc == 2)
	{
		k = strtod(argv[1], NULL);
	}

	for (i = 0; i < k; i++)
	{
		num[i] = i;
	}

	if ((key = ftok(pathname, 0)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}

	if ((msqid = msgget(key, 0666 | IPC_CREAT)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}


	mybuf = (struct mymsgbuf *)malloc(sizeof(struct mymsgbuf));
		
	mybuf->mtype = 1;

	if ((msgsnd(msqid, (struct msgbuf *) mybuf, 2, 0)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}
	printf("SENT\n");


	for (i = 0; i < k; i++)
	{
		if ((result = pthread_create(&thid[i], (pthread_attr_t *)NULL, my_thread, &num[i])) != 0)
		{
			perror(argv[0]);
			exit(errno);
		}
	}

	if ((msgrcv(msqid, (struct msgbuf *) mybuf, 2, 255, 0)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}
	printf("RECEIVED\n");


	for (i = 0; i < k; i++)
	{
		pthread_join(thid[i], (void **)NULL);
	}

	
	return 0;
}
