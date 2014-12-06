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
	int fd_srv = -1, fd_fifo = -1;
	const char name_srv [] = "srv.fifo";
	char * name_fifo, * package, * buf;
	int pid, size;

	if (argc < 2)
	{
		printf("Wrong number of arguments\n");
		exit(-1);
	}

	// Creating client's FIFO

	if ((pid = getpid()) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}

//	printf("PID: %d\n", pid);

	name_fifo = (char *)malloc(15 * sizeof(char));

	if (name_fifo == NULL)
	{
		printf("Can't allocate memory\n");
		exit(-1);
	}

	sprintf(name_fifo, "%d.fifo", pid);
//	printf("FIFO name: %s\n", name_fifo);

	if ((mkfifo(name_fifo, 0666)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}

	if ((fd_fifo = open(name_fifo, O_RDWR)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}
	
	// Opening service FIFO

	// Form package
	
	package = (char *)malloc(256 * sizeof(char));
	if (package == NULL)
	{
		printf("Can't allocate memory\n");
		exit(-1);
	}

	*(int *)package = pid;
	strcpy(package + 4, argv[1]);

//	printf("%s\n", argv[1]);
//	printf("Package: %s\n", package + 4);

	// Send package
	
	if ((fd_srv = open(name_srv, O_WRONLY)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}

//	printf("Service FIFO opened\n");
	
	if ((write(fd_srv, package, 256)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}

//	printf("Package sended\n");
	sleep(1);

	// Read result
	
	buf = (char *)malloc(4096 * sizeof(char));
	if (buf == NULL)
	{
		printf("Can't allocate memory\n");
		exit(-1);
	}
		
	while(1)
	{
		if ((size = read(fd_fifo, buf, 4096)) < 0)
		{
			perror(argv[0]);
			exit(errno);
		}
//		sleep(1);
		if ((write(1, buf, size)) < 0)
		{
			perror(argv[0]);
			exit(errno);
		}

		if (size < 4096)
		{
			break;
		}
		
	}

	close(fd_fifo);
	unlink(name_fifo);
	free(name_fifo);
	free(package);
	free(buf);
	close(fd_srv);
	return 0;
}
