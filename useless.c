/*  
 *  hello-4.c - Demonstrates module documentation.
 */
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

#include "useless.h"

#define DRIVER_AUTHOR "Amihai Neiderman"
#define DRIVER_DESC   "A useless file restorer"

#define DEVICE_MINOR 1


void handle_ioctl_restore_file(struct useless_restore_req req);
struct dentry* find_inode(struct fdtable *fdt, unsigned int inode);


struct useless {
	dev_t dev;
	struct cdev cdev;
} useless;

int useless_open(struct inode *inode, struct file *file)
{
	return 0;
}

int useless_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	int result = 0;
	struct useless_restore_req req;


	switch (ioctl_num) 
	{
		case IOCTL_RESTORE_FILE:
		{
			result = copy_from_user(&req, (void*)ioctl_param, sizeof(struct useless_restore_req));

			if (result)
			{
				printk(KERN_ERR "Your ioctl sucks\n");
			}
	
			handle_ioctl_restore_file(req);
		}
	}
	return 0;
}

struct dentry* find_inode(struct fdtable *fdt, unsigned int inode)
{
	int i, j;
	struct dentry *d;
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
					if (inode == file->f_path.dentry->d_inode->i_ino) 
					{
						printk(KERN_INFO "so inode!\n");
						d = file->f_path.dentry;
						return d;
					}
				}
			}
			i++;
			set >>= 1;
		}
		printk(KERN_INFO "\n");
	}
	return 0;
}



struct path* find_path(struct fdtable *fdt, unsigned int inode)
{
	int i, j;
	struct path *d;
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
					if (inode == file->f_path.dentry->d_inode->i_ino) 
					{
						printk(KERN_INFO "so inode!\n");
						d = &file->f_path;
						return d;
					}
				}
			}
			i++;
			set >>= 1;
		}
		printk(KERN_INFO "\n");
	}
	return 0;
}


void handle_ioctl_restore_file(struct useless_restore_req req)
{
	struct task_struct *task;
	struct dentry *dent = 0;
	struct nameidata nd;
	struct filename *to;
	int err;

	printk(KERN_INFO "got restore request! pid=%d, inode=%d\n", req.pid, req.inode);

	for_each_process(task)
	{
		if (task->pid == req.pid) 
		{
			printk(KERN_INFO "wow! such pid!\n");
			break;
		}
	}

	if (task != NULL) 
	{
		spin_lock(&task->files->file_lock);

		dent = find_inode(files_fdtable(task->files), req.inode);
		struct path *p = find_path(files_fdtable(task->files), req.inode);
		

		if (p != NULL) 
		if (dent != NULL) 
		{
			struct dentry *child;
			struct dentry *d;

			printk(KERN_INFO "inode file name: %s\n", dent->d_name.name);
			printk(KERN_INFO "inode parent name: %s\n", dent->d_parent->d_name.name);

//			char *name = "blah";
//			printk(KERN_INFO "fname:%s\n", name);
//			err = vfs_path_lookup(dent->d_parent, p->mnt,name, LOOKUP_PARENT, &nd);
//			printk(KERN_INFO "lookup:%d, depth:%d\n", err, nd.depth);

//			printk(KERN_INFO "nd path name: %s, inode:%d\n", nd.path.dentry->d_name.name, nd.path.dentry->d_inode->i_ino);

			d = d_alloc_name(dent->d_parent,"RESTORED");

			dent->d_parent->d_inode->i_op->lookup(dent->d_parent->d_inode, d,0);

			inc_nlink(dent->d_inode);
			err = dent->d_parent->d_inode->i_op->link(dent, dent->d_parent->d_inode, d);

			printk(KERN_INFO "link:%d\n", err);
			printk(KERN_INFO "link func: %pF\n", dent->d_parent->d_inode->i_op->link);


// 			list_for_each_entry(child, &dent->d_parent->d_subdirs, d_u.d_child)
//    				printk(KERN_INFO "pname:%s", child->d_name.name);
//			printk(KERN_INFO "------------\n");   			

		}
		mutex_unlock(&nd.path.dentry->d_inode->i_mutex);
		spin_unlock(&task->files->file_lock);
	}

}

int useless_release(struct inode *inode, struct file *file)
{
	return 0;
}

struct file_operations useless_fops = {
	.owner = THIS_MODULE,
	.ioctl = useless_ioctl,
	.open = useless_open,
	.release = useless_release,
};


static int __init init_useless(void)
{
	int result = 0;

	printk(KERN_INFO "initializing useless module\n");

	result = alloc_chrdev_region(&useless.dev, DEVICE_MINOR, 1, "useless");

	if (result < 0) 
	{
		printk(KERN_WARNING "can't get device number for useless module!\n");
		return result;
	}

	cdev_init(&useless.cdev, &useless_fops);
	useless.cdev.owner = THIS_MODULE;		
	result = cdev_add(&useless.cdev, MKDEV(MAJOR(useless.dev), MINOR(useless.dev)), 1);
	/* Fail gracefully if need be */

	if (result)
		printk(KERN_NOTICE "Error %d adding useless device", result);

	return 0;
}

static void __exit cleanup_useless(void)
{
	printk(KERN_INFO "useless exit\n");

	cdev_del(&useless.cdev);

	unregister_chrdev_region(useless.dev, 1);
}

module_init(init_useless);
module_exit(cleanup_useless);

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
