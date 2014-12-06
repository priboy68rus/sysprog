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

void * my_thread(void *a)
{
	int i = *(int *)a;
	printf("NUM: %d THID: %d\n", i, (int)pthread_self());

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
	printf("msqid: %d\n", msqid);

	mybuf->mtype = 1;

	if ((msgsnd(msqid, (struct msgbuf *) mybuf, 2, 0)) < 0)
	{
		exit(errno);
	}
	printf("SENT\n");
}


int main(int argc, char *argv[], char *envp[])
{
	key_t key;
	int msqid;
	const int k = 10;
	pthread_t thid[k];
	int i = 0;
	int result = 0;

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
	printf("msqid: %d\n", msqid);


	for (i = 0; i < k; i++)
	{
		if ((msgrcv(msqid, (struct msgbuf *) mybuf, 2, 0, 0)) < 0)
		{
			perror(argv[0]);
			exit(errno);
		}

		printf("RECEIVED\n");


		if ((result = pthread_create(&thid[i], (pthread_attr_t *)NULL, my_thread, &i)) != 0)
		{
			perror(argv[0]);
			exit(errno);
		}

	}

	if ((msgrcv(msqid, (struct msgbuf *) mybuf, 2, 0, 0)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}


	for (i = 0; i < k; i++)
	{
		pthread_join(thid[i], (void **)NULL);
	}

	
	return 0;
}
