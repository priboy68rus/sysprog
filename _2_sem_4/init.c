#include "shm.h"


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
