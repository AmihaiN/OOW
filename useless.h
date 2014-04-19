#define IOCTL_RESTORE_FILE 0

struct useless_restore_req
{
	unsigned int pid;
	unsigned int inode;
};

