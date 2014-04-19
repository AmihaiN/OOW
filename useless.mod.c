#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x14522340, "module_layout" },
	{ 0x42e80c19, "cdev_del" },
	{ 0xc45a9f63, "cdev_init" },
	{ 0x973873ab, "_spin_lock" },
	{ 0x1eadedc0, "vfs_path_lookup" },
	{ 0x3758301, "mutex_unlock" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0xea147363, "printk" },
	{ 0xb4390f9a, "mcount" },
	{ 0xa6d1bdca, "cdev_add" },
	{ 0xd258dfc9, "init_task" },
	{ 0x133f4331, "d_alloc_name" },
	{ 0x3302b500, "copy_from_user" },
	{ 0x29537c9e, "alloc_chrdev_region" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "E58DFB3AD325D68378FCF82");

static const struct rheldata _rheldata __used
__attribute__((section(".rheldata"))) = {
	.rhel_major = 6,
	.rhel_minor = 5,
};
