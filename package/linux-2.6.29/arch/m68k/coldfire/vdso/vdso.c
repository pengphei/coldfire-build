/*
 * arch/m68k/coldfire/vdso/vdso.c
 *
 * Based on arch/sh/kernel/vsyscall/vsyscall.c
 *
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Kurt Mahan <kmahan@freescale.com>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gfp.h>
#include <linux/module.h>
#include <linux/elf.h>
#include <linux/sched.h>
#include <linux/err.h>

/* Mapping vDSO at the default address (what would've been returned
 * if VDSO_MBASE was 0) makes it impossible to extend data segment
 * (through brk()) for static binaries.  The vDSO fits into one page,
 * so map it just before TASK_UNMAPPED_BASE.
 */
#define VDSO_MBASE (TASK_UNMAPPED_BASE - PAGE_SIZE)

unsigned int vdso_enabled = 1;
EXPORT_SYMBOL_GPL(vdso_enabled);

static struct page *vdso_pages[1];

extern const char vdso_bin_start, vdso_bin_end;

int __init vdso_init(void)
{
	void *vdso_page = (void *)get_zeroed_page(GFP_ATOMIC);
	vdso_pages[0] = virt_to_page(vdso_page);

	printk(KERN_INFO "** VDSO_INIT\n");

	/* copy dso bin in */
	memcpy(vdso_page,
	       &vdso_bin_start, &vdso_bin_end - &vdso_bin_start);

	return 0;
}

/* setup VMA at program startup for the vdso page */
int arch_setup_additional_pages(struct linux_binprm *bprm,
				int executable_stack)
{
	struct mm_struct *mm = current->mm;
	unsigned long addr;
	int ret;

	current->mm->context.vdso = 0;

	down_write(&mm->mmap_sem);
	addr = get_unmapped_area(NULL, VDSO_MBASE, PAGE_SIZE, 0, 0);
	if (IS_ERR_VALUE(addr)) {
		ret = addr;
		goto up_fail;
	}

	ret = install_special_mapping(mm, addr, PAGE_SIZE,
				VM_READ | VM_EXEC |
				VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC |
				VM_ALWAYSDUMP,
				vdso_pages);

	if (unlikely(ret))
		goto up_fail;

	current->mm->context.vdso = (void *)addr;

up_fail:
#ifdef DEBUG
	printk(KERN_DEBUG "arch_setup_additional_pages: addr: %lx; ret: %d\n",
		addr, ret);
#endif

	up_write(&mm->mmap_sem);
	return ret;
}

/*
 * check vma name
 */
const char *arch_vma_name(struct vm_area_struct *vma)
{
	if (vma->vm_mm && vma->vm_start == (long)vma->vm_mm->context.vdso)
		return "[vdso]";

	return NULL;
}

struct vm_area_struct *get_gate_vma(struct task_struct *task)
{
	return NULL;
}

int in_gate_area(struct task_struct *task, unsigned long address)
{
	return 0;
}

int in_gate_area_no_task(unsigned long address)
{
	return 0;
}
