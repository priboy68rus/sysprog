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

int main(int argc, char *argv[], char *envp[])
{
	printf("1");
	int pid;
	char * name_file, * package, * buf;
	char name_fifo [15], name_srv [] = "srv.fifo";
	int fd_fifo = -1, fd_srv = -1;

	printf("1");
	if (argc < 2)
	{
		printf("Wrong number of arguments\n");
		exit(-1);
	}	

	printf("1");
	name_file = argv[1];
	printf("1");
	pid = getpid();

	printf("1");
	if ((fd_srv = open(name_srv, O_WRONLY)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}

	printf("Service FIFO opened\n");

	sprintf(name_fifo, "%d.fifo", pid);

	if ((mkfifo(name_fifo, 0666)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}

	if ((fd_fifo = open(name_fifo, O_RDONLY)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}

	package = (char *)malloc(256 * sizeof(char));
	if (package == NULL)
	{
		printf("Can't allocate memory\n");
		exit(-1);
	}

	*(int *)package = pid;
	sprintf(package + 4, "%s", name_file);

	printf("%s\n", package);

	if ((write(fd_srv, package, 256)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}

	return 0;
}
