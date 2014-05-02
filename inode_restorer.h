#define IOCTL_RESTORE_FILE 0

struct inode_restorer_restore_req
{
	unsigned int pid;
	unsigned long inode;
};

