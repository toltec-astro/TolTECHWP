#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xa4b86400, "module_layout" },
	{ 0x5ab5b891, "param_ops_int" },
	{ 0x3b67ff49, "pci_unregister_driver" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x74007fb7, "class_destroy" },
	{ 0xfbfddd92, "__pci_register_driver" },
	{ 0xcaed4f8, "__class_create" },
	{ 0x541fd03b, "__register_chrdev" },
	{ 0x2072ee9b, "request_threaded_irq" },
	{ 0x8b58c8f8, "device_create" },
	{ 0xd2198226, "cdev_add" },
	{ 0xb5739c63, "cdev_init" },
	{ 0x2b6d8feb, "cdev_alloc" },
	{ 0x93a219c, "ioremap_nocache" },
	{ 0x85bd1608, "__request_region" },
	{ 0x3f115012, "dma_set_mask" },
	{ 0x97b51df7, "pci_set_master" },
	{ 0x54836bf2, "pci_enable_device" },
	{ 0x977f511b, "__mutex_init" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x8ddd8aad, "schedule_timeout" },
	{ 0x1000e51, "schedule" },
	{ 0xb44ad4b3, "_copy_to_user" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0xf1969a8e, "__usecs_to_jiffies" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0x3eeb2322, "__wake_up" },
	{ 0x3812050a, "_raw_spin_unlock_irqrestore" },
	{ 0x51760917, "_raw_spin_lock_irqsave" },
	{ 0x30e4ecda, "remap_pfn_range" },
	{ 0xb665f56d, "__cachemode2pte_tbl" },
	{ 0x445a81ce, "boot_cpu_data" },
	{ 0xc5850110, "printk" },
	{ 0xb3a666c0, "kmem_cache_alloc_trace" },
	{ 0x31ca542f, "kmalloc_caches" },
	{ 0x409bcb62, "mutex_unlock" },
	{ 0x2ab7989d, "mutex_lock" },
	{ 0x1035c7c2, "__release_region" },
	{ 0x77358855, "iomem_resource" },
	{ 0xf38a3177, "pci_disable_device" },
	{ 0x37a0cba, "kfree" },
	{ 0x725a8218, "cdev_del" },
	{ 0xf6cb2bfb, "device_destroy" },
	{ 0xedc03953, "iounmap" },
	{ 0xc1514a3b, "free_irq" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("pci:v000010B5d00009056sv00006000sd00000826bc*sc*i*");

MODULE_INFO(srcversion, "39309B1A67E27043B6D4E05");
