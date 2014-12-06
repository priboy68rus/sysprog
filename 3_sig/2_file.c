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

char * ans;
int num = 0;
int cur_len = 0;

void handler_1(int nsig)
{
	ans[cur_len] = ans[cur_len] | (1 << num);
	num++;
	signal(SIGUSR2, handler_1);	
}

void handler_0(int nsig)
{
	num++;
	signal(SIGUSR1, handler_0);
}

void handler(int nsig)
{
	signal(SIGUSR1, handler);
}



int main(int argc, char *argv[], char *envp[])
{
	if (argc < 2)
	{
		printf("Wrong number of arguments\n");
		exit(-1);
	}

	int pid_fork = -1;
	int fd = -1;
	int i = 0;
	int size = 0;
	int len = strlen(argv[1]);
	char sym = argv[1][0];

	ans = (char *)malloc(len * sizeof(char));
	for (i = 0; i < len; i++)
	{
		ans[i] = 0;
	}



	sigset_t mask1, mask2, oldmask;

	sigemptyset(&mask1);
	sigaddset(&mask1, SIGUSR1);
	sigaddset(&mask2, SIGUSR2);

	sigemptyset(&mask2);
	sigaddset(&mask2, SIGUSR1);

	signal(SIGUSR1, handler);

	sigprocmask(SIG_BLOCK, &mask2, &oldmask);

	
	if ((pid_fork = fork()) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}
	else if (pid_fork == 0)
	{
		//Child (receives)

		signal(SIGUSR1, handler_0);
		signal(SIGUSR2, handler_1);
		sigprocmask(SIG_BLOCK, &mask1, NULL);

		kill(getppid(), SIGUSR1);

		while (1)
		{
			sigsuspend(&oldmask);

			if (num == 8)
			{
				num = 0;
				write(1, &ans[cur_len], 1);
				cur_len++;
			}

			kill(getppid(), SIGUSR1);
		}
		
	}
	else
	{
		//Parent (sends)

		i = 0;
		int reread = 0;

		if ((fd = open(argv[1], O_RDONLY)) < 0)
		{
			perror(argv[0]);
			exit(errno);
		}

		char * buf = (char *)malloc(4096 * sizeof(char));
		if (buf == NULL)
		{
			printf("Cant allocate memory\n");
			exit(-1);
		}

		if ((size = read(fd, buf, 4096)) < 0)
		{
			perror(argv[0]);
			exit(errno);
		}

		if (size < 4096)
			reread = 0;
		else
			reread = 1;

		len = strlen(buf);

		while (1)
		{
			sigsuspend(&oldmask);

			if (i == 8)
			{
				cur_len++;
				i = 0;

				if (cur_len == len)
				{
					if (reread == 0)
					{
						kill(pid_fork, SIGKILL);
						exit(0);
					}
					else
					{
						cur_len = 0;

						if ((size = read(fd, buf, 4096)) < 0)
						{
							perror(argv[0]);
							exit(errno);
						}

						if (size < 4096)
							reread = 0;
						else
							reread = 1;
					}
				}

				// sym = argv[1][cur_len];
				sym = buf[cur_len];

			}
			if ((sym >> i & 1) == 0)
				kill(pid_fork, SIGUSR1);
			else
				kill(pid_fork, SIGUSR2);


			i++;
		}
	}

	return 0;
}
