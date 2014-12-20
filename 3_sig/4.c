#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


#define SIZE 256

/*parent's pid*/
pid_t pid;
/*child's pid*/
int id;
pid_t cid;
int nbuf;
char * buf;

sigset_t mask1, mask2, oldmask;

int written_bit = 0;

void handler0(int nsig)
{
	written_bit = 0;
	signal(SIGUSR1, handler0);
}

void handler1(int nsig)
{
	written_bit = 1;
	signal(SIGUSR2, handler1);
}

void phandler(int nsig)
{
	signal(SIGUSR1, phandler);
}

void handler(int nsig)
{
	write(1, buf, nbuf);
	exit(0);
}

int main(int argc, char* argv[]) 
{
	int nbit_i = 0;
	char buff;
	char out = 0;
	int nbit_o = 0;
	pid = getpid();
	signal(SIGUSR1, phandler);

	if (argc < 2) {
		fprintf(stderr, "Not enough args!\n");
		exit(-1);
	}

	// buff = argv[1][0];

	sigemptyset(&mask1);
	sigaddset(&mask1, SIGUSR1);
	sigaddset(&mask1, SIGUSR2);
	sigaddset(&mask1, SIGINT);

	sigemptyset(&mask2);
	sigaddset(&mask2, SIGUSR1);

	/* "block" other signals for receive USR1 */
	sigprocmask(SIG_BLOCK, &mask2, &oldmask);

	if ((id = fork()) < 0) {
		perror("fork failed");
		exit(errno);
	}
	if (id > 0) {	/* parent */
		fflush(stdout);
		int fd = -1;
		if ((fd = open(argv[1], O_RDONLY, 0666)) < 0)
		{
			printf("Cant open file\n");
			exit(-1);
		}
		int res = 1;
		read(fd, &buff, 1);
		do {
			sigsuspend(&oldmask);
			(buff & 1 << nbit_i) ? \
				kill(id, SIGUSR2) : kill(id, SIGUSR1);
			/* all bits were sent */
			if (nbit_i == sizeof(char) * 8 - 1)
			{
				nbit_i = 0;
				if (res == 0)
				{
					kill(id, SIGINT);
					fpurge(stdout);
					exit(0);
				}
				res = read(fd, &buff, 1);
			}
			else
			{
				nbit_i++;
			}
		} while (1);
	}
	else 
	{		/* child */
		buf = (char *)malloc(SIZE * sizeof(int));
		nbuf = 0;
		fflush(stdout);
		signal(SIGINT, handler);
		signal(SIGUSR1, handler0);	/*setting handlers*/
		signal(SIGUSR2, handler1);
		sigprocmask(SIG_BLOCK, &mask1, NULL);
		cid = getpid();		/*child's pid*/
		kill(pid, SIGUSR1);	/*telling hi to parent*/
		do {			/*waiting for signals*/
			sigsuspend(&oldmask);
			if (written_bit == 1) 
				out = (1 << nbit_o) | out;
			nbit_o++;
			/* all bits were received */
			if (nbit_o == sizeof(char) * 8) {
				// fprintf(stdout, "%c", out);
				// printf("%c", out);
				// write(1, &out, 1);
				buf[nbuf] = out;
				nbit_o = 0;
				nbuf++;
				if (nbuf == SIZE)
				{
					write(1, buf, SIZE);
					nbuf = 0;
				}
				out = 0;
			}
			kill(pid, SIGUSR1);
		} while (1);
	}

	return 0;
}

