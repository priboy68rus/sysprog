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
#include <signal.h>

void handler_0(int nsig)
{
	signal(SIGUSR1, handler_0);
}

void handler_1(int nsig)
{
	signal(SIGUSR2, handler_1);
}

int main(int argc, char *argv[], char *envp[])
{
	if (argc < 2)
	{
		printf("Wrong number of arguments\n");
		exit(-1);
	}

	int pid_fork = -1;

	sigset_t mask1, mask2, oldmask;
	sigemptyset(&mask1);
	sigaddset(&mask1, SIGUSR1);
	sigaddset(&mask1, SIGUSR2);
	sigemptyset(&mask2);
	sigaddset(&mask2, SIGUSR1);

	sigprocmask(SIG_BLOCK, &mask2, &oldmask);

	if ((pid_fork = fork()) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}
	else if (pid_fork == 0)
	{
		//Child
		signal(SIGUSR1, handler_0);
		signal(SIGUSR2, handler_1);
		sigprocmask(SIG_BLOCK, &mask1, NULL);

		while(1)
		{
			sigsuspend(&oldmask);
			//write

			kill(getppid(), SIGUSR1);

		}

	}
	else
	{
		//Parent
		int fd = -1;

		if ((fd = open(argv[1], O_RDONLY, 0666)) < 0)
		{
			printf("Can't open file\n");
			exit(-1);
		}

		
		while(1)
		{
			sigsuspend(&oldmask);
			//send
			if ()
			{
				kill(pid_fork, SIGKILL);
				exit(0);
			}
			//next
		}
	}


	return 0;
}
