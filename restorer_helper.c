#include "inode_restorer.h"

#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdlib.h>
#include <stdio.h>

int main(int argc, const char* argv[])
{
	int fd;
	struct inode_restorer_restore_req req;


	fd = open("inode_restorer", 0);

	if (fd < 0) 
	{
		printf("couldn't open the file\n");
		return 0;
	}

	if (argc > 2) 
	{
		req.pid = atoi(argv[1]);
		req.inode = atoi(argv[2]);
	}
	else if (argc == 2) 
	{
		req.pid = getpid();
		req.inode = atoi(argv[1]);
	}
	else
	{
		return;
	}

	return ioctl(fd, IOCTL_RESTORE_FILE, &req);
}
