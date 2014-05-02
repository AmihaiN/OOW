#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */
#include <linux/fs.h>       /* device numbers 	*/
#include <linux/cdev.h>
#include <linux/uaccess.h>  /* copy from user */
#include <linux/sched.h>
#include <linux/dcache.h>
#include <linux/namei.h>
#include <linux/fdtable.h>

#include "inode_restorer.h"

#define DRIVER_AUTHOR "Ido Ben-Yair & Amihai Neiderman"
#define DRIVER_DESC   "A inode_restorer file restorer"

#define DEVICE_MINOR 1

int handle_ioctl_restore_file(struct inode_restorer_restore_req req);
struct dentry* fdtable_find_inode(struct fdtable *fdt, unsigned long inode);
struct dentry* task_find_file(struct task_struct *task, unsigned long inode);
int dentry_restore(struct dentry *dentry);

struct inode_restorer {
	dev_t dev;
	struct cdev cdev;
} inode_restorer;

int inode_restorer_open(struct inode *inode, struct file *file)
{
	return 0;
}

int inode_restorer_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	int result = 0;
	struct inode_restorer_restore_req req;

	switch (ioctl_num) 
	{
	case IOCTL_RESTORE_FILE:
		result = copy_from_user(&req, (void*)ioctl_param, sizeof(struct inode_restorer_restore_req));

		if (result)
		{
			printk(KERN_DEBUG "Unable to copy request parameters from userspace!\n");
			return -EFAULT;
		}

		return handle_ioctl_restore_file(req);

	default:
		return -EFAULT;
	}
}

// Returns with incremented reference to dentry
struct dentry* fdtable_find_inode(struct fdtable *fdt, unsigned long inode)
{
	int i, j;
	j = 0;

	for (;;) 
	{
		unsigned long set;
		i = j * __NFDBITS;
		if (i >= fdt->max_fds)
			break;
		set = fdt->open_fds->fds_bits[j++];
		while (set) 
		{
			if (set & 1) 
			{
				struct file *file = rcu_dereference(fdt->fd[i]);
				if (file) 
				{
					struct dentry *dentry = dget(file->f_path.dentry);

					if (inode == dentry->d_inode->i_ino) 
					{
						printk(KERN_DEBUG "Found inode #%lu!\n", dentry->d_inode->i_ino);
						return dentry;
					}

					dput(dentry);
				}
			}
			i++;
			set >>= 1;
		}
	}
	return 0;
}

int handle_ioctl_restore_file(struct inode_restorer_restore_req req)
{
	struct task_struct *task = NULL;
	struct dentry *dentry = NULL;
	int retval = 0;

	printk(KERN_DEBUG "Got file restore request! pid=%du, inode=%lu\n", req.pid, req.inode);

	//read_lock(&tasklist_lock);

	for_each_process(task)
		if (task->pid == req.pid) 
		{
			printk(KERN_DEBUG "Found process!\n");
			break;
		}

	//read_unlock(&tasklist_lock);

	if (task) 
	{
		get_task_struct(task);
		dentry = task_find_file(task, req.inode);
		put_task_struct(task);
	}

	if (dentry)
	{
		retval = dentry_restore(dentry);
		dput(dentry);
		return retval;
	}

	return -EFAULT;
}

struct dentry* task_find_file(struct task_struct *task, unsigned long inode)
{
	struct files_struct *files = NULL;
	struct dentry *dentry = NULL;

	files = task->files;
	atomic_inc(&files->count);

	if (!files) 
		return NULL;

	dentry = fdtable_find_inode(files_fdtable(task->files), inode);

	//mutex_unlock(&nd.path.dentry->d_inode->i_mutex);
	atomic_dec(&files->count);
	return dentry;
}

int dentry_restore(struct dentry *dentry)
{
	int err = 0;
	if (dentry != NULL) 
	{
		struct dentry *d;

		printk(KERN_DEBUG "inode file name: %s\n", dentry->d_name.name);
		printk(KERN_DEBUG "inode parent name: %s\n", dentry->d_parent->d_name.name);

		d = d_alloc_name(dentry->d_parent, "RESTORED");

		dentry->d_parent->d_inode->i_op->lookup(dentry->d_parent->d_inode, d, 0);

		// Increment the link count so that link() works, perform the link and then
		// Decrement it back, marking the inode as dirty.
		inc_nlink(dentry->d_inode);
		err = dentry->d_parent->d_inode->i_op->link(dentry, dentry->d_parent->d_inode, d);
		drop_nlink(dentry->d_inode);

		return err;
	}

	return -EFAULT;
}

int inode_restorer_release(struct inode *inode, struct file *file)
{
	return 0;
}

struct file_operations inode_restorer_fops = {
	.owner = THIS_MODULE,
	.ioctl = inode_restorer_ioctl,
	.open = inode_restorer_open,
	.release = inode_restorer_release,
};


static int __init init_inode_restorer(void)
{
	int result = 0;

	printk(KERN_INFO "initializing inode_restorer module\n");

	result = alloc_chrdev_region(&inode_restorer.dev, DEVICE_MINOR, 1, "inode_restorer");

	if (result < 0)
	{
		printk(KERN_WARNING "can't get device number for inode_restorer module!\n");
		return result;
	}

	cdev_init(&inode_restorer.cdev, &inode_restorer_fops);
	inode_restorer.cdev.owner = THIS_MODULE;		
	result = cdev_add(&inode_restorer.cdev, MKDEV(MAJOR(inode_restorer.dev), MINOR(inode_restorer.dev)), 1);
	/* Fail gracefully if need be */

	if (result)
		printk(KERN_NOTICE "Error %d adding inode_restorer device", result);

	return 0;
}

static void __exit cleanup_inode_restorer(void)
{
	printk(KERN_INFO "inode_restorer exit\n");

	cdev_del(&inode_restorer.cdev);

	unregister_chrdev_region(inode_restorer.dev, 1);
}

module_init(init_inode_restorer);
module_exit(cleanup_inode_restorer);

/*  
 *  You can use strings, like this:
 */

/* 
 * Get rid of taint message by declaring code as GPL. 
 */
MODULE_LICENSE("GPL");

/*
 * Or with defines, like this:
 */
MODULE_AUTHOR(DRIVER_AUTHOR);	/* Who wrote this module? */
MODULE_DESCRIPTION(DRIVER_DESC);	/* What does this module do */
