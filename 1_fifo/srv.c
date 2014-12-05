#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

void handler(int nsig)
{
	printf("Removing service FIFO...\n");
	unlink("srv.fifo");
	printf("Exiting...\n");
	exit(0);
}

int main(int argc, char *argv[], char *envp[])
{
	char name_srv [] = "srv.fifo";
	int size = 0, fd_srv = -1, fd_file = -1, fd_fifo = -1, id = -1;
	char * package, * buf;
	char name_fifo[15];

	signal(SIGINT, handler);
	
	// Create service FIFO

	if ((mkfifo(name_srv, 0666)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}

	if ((fd_srv = open(name_srv, O_RDWR)) < 0)
	{
		perror(argv[0]);
		exit(errno);
	}

	printf("FIFO created and opened\n");

	package = (char *)malloc(256 * sizeof(char));
	if (package == NULL)
	{
		printf("Can't allocate memory\n");
		exit(-1);
	}

	buf = (char *)malloc(4096 * sizeof(char));
	if (buf == NULL)
	{
		printf("Can't allocate memory\n");
		exit(-1);
	}

	// Loop part
	while(1)
	{
		if ((size = read(fd_srv, package, 256)) < 0)
		{
			perror(argv[0]);
			exit(errno);
		}

		printf("Pong!\n");
		
		id = *(int *)package;

		sprintf(name_fifo, "%d.fifo", id);

		printf("%s\n", name_fifo);
	/*
		if ((mkfifo(name_fifo,0666)) < 0)
		{
			perror(argv[0]);
			exit(errno);
		}
	*/
		if ((fd_fifo = open(name_fifo, O_WRONLY)) < 0)
		{
			perror(argv[0]);
			exit(errno);
		}

		printf("Client FIFO opened\n");

		if ((fd_file = open((package + 4), O_RDONLY)) < 0)
		{
			perror(argv[0]);
			exit(errno);
		}

		printf("File opened\n");

		while(1)
		{
			if ((size = read(fd_file, buf, 4096)) < 0)
			{
				perror(argv[0]);
				exit(errno);
			}

			if (size == 0)
			{
				break;
			}

			if ((write(fd_fifo, buf, size)) < 0)
			{
				perror(argv[0]);
				exit(errno);
			}
		}
		close(fd_fifo);
		close(fd_file);
	}

	free(buf);
	free(package);
		
	return 0;
}
