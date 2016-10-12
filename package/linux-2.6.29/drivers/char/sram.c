/*
 * linux/drivers/char/sram.c
 *
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Author: Lanttor.Guo@freescale.com
 *
 * SRAM char device driver, implements mmap() system call to provide
 * interface for user space access.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/page.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/autoconf.h>

MODULE_LICENSE("GPL");

static struct class *sram_class;
static int sram_major;
static int sram_minor;
static struct cdev sram_cdev;

/*
 * Set up the cdev structure for sram.
 */
static void sram_setup_cdev(struct cdev *dev, int minor,
			    struct file_operations *fops)
{
	int err, devno = MKDEV(sram_major, minor);

	cdev_init(dev, fops);
	dev->owner = THIS_MODULE;
	dev->ops = fops;
	err = cdev_add(dev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding sram%d", err, minor);
}

static int sram_open(struct inode *inode, struct file *filp)
{
	filp->f_mapping->backing_dev_info = &directly_mappable_cdev_bdi;
	return 0;
}

static int sram_release(struct inode *inode, struct file *filp)
{
	return 0;
}

void sram_vma_open(struct vm_area_struct *vma)
{
	printk(KERN_DEBUG "Sram VMA open, virt %lx, phys %lx\n",
			   vma->vm_start,
			   CONFIG_SRAM_BASE + (vma->vm_pgoff << PAGE_SHIFT));
}

void sram_vma_close(struct vm_area_struct *vma)
{
	printk(KERN_DEBUG "Sram VMA close.\n");
}


static struct vm_operations_struct sram_remap_vm_ops = {
	.open =  sram_vma_open,
	.close = sram_vma_close,
};

static int sram_remap_mmap(struct file *filp, struct vm_area_struct *vma)
{
	size_t size = vma->vm_end - vma->vm_start;

	if (PAGE_ALIGN(size) > CONFIG_SRAM_SIZE) {
		printk(KERN_ERR "required length exceed the size "
		       "of physical sram (%x)\n", CONFIG_SRAM_SIZE);
		return -EAGAIN;
	}

	if ((CONFIG_SRAM_BASE + (vma->vm_pgoff << PAGE_SHIFT) + size)
	    > (CONFIG_SRAM_BASE + CONFIG_SRAM_SIZE)) {
		printk(KERN_ERR "required sram range exceed the size "
		       "of phisical sram\n");
		return -EAGAIN;
	}

	if (remap_pfn_range(vma, vma->vm_start,
			    (CONFIG_SRAM_BASE >> PAGE_SHIFT) + vma->vm_pgoff,
			    size,
			    vma->vm_page_prot)) {
		printk(KERN_ERR "remap_pfn_range faile at %s()\n", __func__);
		return -EAGAIN;
	}

	vma->vm_ops = &sram_remap_vm_ops;
	return 0;
}

static struct file_operations sram_ops = {
	.owner   = THIS_MODULE,
	.open    = sram_open,
	.release = sram_release,
	.mmap    = sram_remap_mmap,
};

static int __init sram_chrdev_init(void)
{
	int minor_devs = 1;
	int result;
	dev_t dev = 0;
	sram_minor = 0;

	result = alloc_chrdev_region(&dev, sram_minor, minor_devs, "sram");
	sram_major = MAJOR(dev);
	if (result < 0) {
		printk(KERN_WARNING "sram: can't get major %d\n", sram_major);
		return result;
	}

	sram_setup_cdev(&sram_cdev, 0, &sram_ops);

	sram_class = class_create(THIS_MODULE, "sram");
	device_create(sram_class, NULL, MKDEV(sram_major, sram_minor),
		      NULL, "sram");

	return 0;
}

static void sram_chrdev_cleanup(void)
{
	cdev_del(&sram_cdev);
	device_destroy(sram_class, MKDEV(sram_major, sram_minor));
	class_destroy(sram_class);
	unregister_chrdev_region(MKDEV(sram_major, 0), 1);
}

module_init(sram_chrdev_init);
module_exit(sram_chrdev_cleanup);
