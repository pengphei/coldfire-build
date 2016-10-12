/*
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Author: Lanttor.Guo@freescale.com
 *
 * Providing on-chip SRAM allocation and free APIs to kernel
 * The implemention uses gen_pool_alloc/gen_pool_free interface
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/genalloc.h>

/* sram memory min allocation size per once */
static long blk_size = CONFIG_SRAM_ALLOC_GRANULARITY;
static struct gen_pool *sram_pool;

/*
 * Set up memory pools to manage on-chip sram.
 * @ start	the start address of SRAM
 * @ size	the size of SRAM
 * return	return 0 express success
 */
int declare_sram_pool(void *start, size_t size)
{
	int status = 0;

	pr_debug("%s %p %d\n", __func__, start, size);

	sram_pool = gen_pool_create(ilog2(blk_size), -1);
	if (!sram_pool) {
		printk(KERN_ERR "gen_pool_creat faile at %s()\n", __func__);
		status = -ENOMEM;
	}

	status = gen_pool_add(sram_pool, (unsigned long)start, size, -1);
	if (status < 0)
		printk(KERN_ERR "gen_pool_add failed at %s()\n", __func__);

	return status;

}

/*
 * Allocate memory from sram pool
 * @ len	the size of allocated memory
 * return	return the start addr of allocated memory
 */
void *sram_alloc(size_t len)
{
	unsigned long vaddr;

	if (!len) {
		printk(KERN_ERR "the len parameter of sram_alloc() is zero\n");
		return NULL;
	}

	vaddr = gen_pool_alloc(sram_pool, len);
	if (!vaddr)
		return NULL;

	return (void *)vaddr;
}
EXPORT_SYMBOL(sram_alloc);

/*
 * Free memory to sram pool
 * @ addr	the addr of allocated memory
 * @ len	the size of allocated memory
 */
void sram_free(void *addr, size_t len)
{
	gen_pool_free(sram_pool, (unsigned long)addr, len);
}
EXPORT_SYMBOL(sram_free);
