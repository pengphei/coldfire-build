/*
 * include/asm-m68k/cf_5445x_cacheflush.h - Coldfire 5445x Cache
 *
 * Copyright 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * Kurt Mahan kmahan@freescale.com
 *
 * Based on include/asm-m68k/cacheflush.h
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#ifndef M68K_CF_5445x_CACHEFLUSH_H
#define M68K_CF_5445x_CACHEFLUSH_H

#include <asm/cfcache.h>

/*
 * Coldfire Cache Model
 *
 * The Coldfire processors use a Harvard architecture cache configured
 * as four-way set associative.  The cache does not implement bus snooping
 * so cache coherency with other masters must be maintained in software.
 *
 * The cache is managed via the CPUSHL instruction in conjunction with
 * bits set in the CACR (cache control register).  Currently the code
 * uses the CPUSHL enhancement which adds the ability to
 * invalidate/clear/push a cacheline by physical address.  This feature
 * is designated in the Hardware Configuration Register [D1-CPES].
 *
 * CACR Bits:
 *	DPI[28]		cpushl invalidate disable for d-cache
 *	IDPI[12]	cpushl invalidate disable for i-cache
 *	SPA[14]		cpushl search by physical address
 *	IVO[20]		cpushl invalidate only
 *
 * Random Terminology:
 *  * invalidate = reset the cache line's valid bit
 *  * push = generate a line-sized store of the data if its contents are
 *	     marked as modifed (the modified flag is cleared after
 *	     the store)
 *  * clear = push + invalidate
 */

/**
 * flush_icache - Flush all of the instruction cache
 */
static inline void flush_icache(void)
{
	asm volatile("nop\n"
		     "moveq%.l	#0,%%d0\n"
		     "moveq%.l	#0,%%d1\n"
		     "move%.l	%%d0,%%a0\n"
		     "1:\n"
		     "cpushl	%%ic,(%%a0)\n"
		     "add%.l	#0x0010,%%a0\n"
		     "addq%.l	#1,%%d1\n"
		     "cmpi%.l	%0,%%d1\n"
		     "bne	1b\n"
		     "moveq%.l	#0,%%d1\n"
		     "addq%.l	#1,%%d0\n"
		     "move%.l	%%d0,%%a0\n"
		     "cmpi%.l	#4,%%d0\n"
		     "bne	1b\n"
		     : : "i" (CACHE_SETS)
		     : "a0", "d0", "d1");
}

/**
 * flush_dcache - Flush all of the data cache
 */
static inline void flush_dcache(void)
{
	asm volatile("nop\n"
		     "moveq%.l	#0,%%d0\n"
		     "moveq%.l	#0,%%d1\n"
		     "move%.l	%%d0,%%a0\n"
		     "1:\n"
		     "cpushl	%%dc,(%%a0)\n"
		     "add%.l	#0x0010,%%a0\n"
		     "addq%.l	#1,%%d1\n"
		     "cmpi%.l	%0,%%d1\n"
		     "bne	1b\n"
		     "moveq%.l	#0,%%d1\n"
		     "addq%.l	#1,%%d0\n"
		     "move%.l	%%d0,%%a0\n"
		     "cmpi%.l	#4,%%d0\n"
		     "bne	1b\n"
		     : : "i" (CACHE_SETS)
		     : "a0", "d0", "d1");
}

/**
 * flush_bcache - Flush all of both caches
 */
static inline void flush_bcache(void)
{
	asm volatile("nop\n"
		     "moveq%.l	#0,%%d0\n"
		     "moveq%.l	#0,%%d1\n"
		     "move%.l	%%d0,%%a0\n"
		     "1:\n"
		     "cpushl	%%bc,(%%a0)\n"
		     "add%.l	#0x0010,%%a0\n"
		     "addq%.l	#1,%%d1\n"
		     "cmpi%.l	%0,%%d1\n"
		     "bne	1b\n"
		     "moveq%.l	#0,%%d1\n"
		     "addq%.l	#1,%%d0\n"
		     "move%.l	%%d0,%%a0\n"
		     "cmpi%.l	#4,%%d0\n"
		     "bne	1b\n"
		     : : "i" (CACHE_SETS)
		     : "a0", "d0", "d1");
}

/**
 * cf_cache_clear - invalidate cache
 * @paddr: starting physical address
 * @len: number of bytes
 *
 * Invalidate cache lines starting at paddr for len bytes.
 * Those lines are not pushed.
 */
static inline void cf_cache_clear(unsigned long paddr, int len)
{
	/* number of lines */
	len = (len + (CACHE_LINE_SIZE-1)) / CACHE_LINE_SIZE;
	if (len == 0)
		return;

	/* align on set boundary */
	paddr &= 0xfffffff0;

	asm volatile("nop\n"
		     "move%.l   %2,%%d0\n"
		     "or%.l	%3,%%d0\n"
		     "movec	%%d0,%%cacr\n"
		     "move%.l	%0,%%a0\n"
		     "move%.l	%1,%%d0\n"
		     "1:\n"
		     "cpushl	%%bc,(%%a0)\n"
		     "lea	0x10(%%a0),%%a0\n"
		     "subq%.l	#1,%%d0\n"
		     "bne%.b	1b\n"
		     "movec	%2,%%cacr\n"
		     : : "a" (paddr), "r" (len),
			 "r" (shadow_cacr),
			 "i" (CF_CACR_SPA+CF_CACR_IVO)
		     : "a0", "d0");
}

/**
 * cf_cache_push - Push dirty cache out with no invalidate
 * @paddr: starting physical address
 * @len: number of bytes
 *
 * Push the any dirty lines starting at paddr for len bytes.
 * Those lines are not invalidated.
 */
static inline void cf_cache_push(unsigned long paddr, int len)
{
	/* number of lines */
	len = (len + (CACHE_LINE_SIZE-1)) / CACHE_LINE_SIZE;
	if (len == 0)
		return;

	/* align on set boundary */
	paddr &= 0xfffffff0;

	asm volatile("nop\n"
		     "move%.l   %2,%%d0\n"
		     "or%.l	%3,%%d0\n"
		     "movec	%%d0,%%cacr\n"
		     "move%.l	%0,%%a0\n"
		     "move%.l	%1,%%d0\n"
		     "1:\n"
		     "cpushl	%%bc,(%%a0)\n"
		     "lea	0x10(%%a0),%%a0\n"
		     "subq%.l	#1,%%d0\n"
		     "bne.b	1b\n"
		     "movec	%2,%%cacr\n"
		     : : "a" (paddr), "r" (len),
			 "r" (shadow_cacr),
			 "i" (CF_CACR_SPA+CF_CACR_DPI+CF_CACR_IDPI)
		     : "a0", "d0");
}

/**
 * cf_cache_flush - Push dirty cache out and invalidate
 * @paddr: starting physical address
 * @len: number of bytes
 *
 * Push the any dirty lines starting at paddr for len bytes and
 * invalidate those lines.
 */
static inline void cf_cache_flush(unsigned long paddr, int len)
{
	/* number of lines */
	len = (len + (CACHE_LINE_SIZE-1)) / CACHE_LINE_SIZE;
	if (len == 0)
		return;

	/* align on set boundary */
	paddr &= 0xfffffff0;

	asm volatile("nop\n"
		     "move%.l   %2,%%d0\n"
		     "or%.l	%3,%%d0\n"
		     "movec	%%d0,%%cacr\n"
		     "move%.l	%0,%%a0\n"
		     "move%.l	%1,%%d0\n"
		     "1:\n"
		     "cpushl	%%bc,(%%a0)\n"
		     "lea	0x10(%%a0),%%a0\n"
		     "subq%.l	#1,%%d0\n"
		     "bne.b	1b\n"
		     "movec	%2,%%cacr\n"
		     : : "a" (paddr), "r" (len),
			 "r" (shadow_cacr),
			 "i" (CF_CACR_SPA)
		     : "a0", "d0");
}

/**
 * cf_cache_flush_range - Push dirty data/inst cache in range out and invalidate
 * @vstart - starting virtual address
 * @vend: ending virtual address
 *
 * Push the any dirty data/instr lines starting at paddr for len bytes and
 * invalidate those lines.
 */
static inline void cf_cache_flush_range(unsigned long vstart, unsigned long vend)
{
	int len;

	/* align on set boundary */
	vstart &= 0xfffffff0;
	vend = PAGE_ALIGN((vend + (CACHE_LINE_SIZE-1))) & 0xfffffff0;
	len = vend - vstart;
	if (len == 0)
		return;
	vstart = __pa(vstart);
	vend = vstart + len;

	asm volatile("nop\n"
		     "move%.l   %2,%%d0\n"
		     "or%.l	%3,%%d0\n"
		     "movec	%%d0,%%cacr\n"
		     "move%.l	%0,%%a0\n"
		     "move%.l	%1,%%a1\n"
		     "1:\n"
		     "cpushl	%%bc,(%%a0)\n"
		     "lea	0x10(%%a0),%%a0\n"
		     "cmpa%.l	%%a0,%%a1\n"
		     "bne.b	1b\n"
		     "movec	%2,%%cacr\n"
		     : /* no return */
		     : "a" (vstart), "a" (vend),
		       "r" (shadow_cacr),
		       "i" (CF_CACR_SPA)
		     : "a0", "a1", "d0");
}

/**
 * cf_dcache_flush_range - Push dirty data cache in range out and invalidate
 * @vstart - starting virtual address
 * @vend: ending virtual address
 *
 * Push/Clear the virtual range *vstart* -> *vend* using the enhanced
 * cpushl instruction to search by physical address.
 *
 * Assumes CACR[IVO] is 0
 *	   CACR[DDPI] is 0
 *
 * Sets CACR[SPA] to 1 for the duration of this routine
 */
static inline void cf_dcache_flush_range(unsigned long vstart,
					 unsigned long vend)
{
	/* align on set boundary */
	vstart &= 0xfffffff0;
	vend = (vend + (CACHE_LINE_SIZE-1)) & 0xfffffff0;

	asm volatile("nop\n"
		     "move%.l   %2,%%d0\n"
		     "or%.l	%3,%%d0\n"
		     "movec	%%d0,%%cacr\n"
		     "move%.l	%0,%%a0\n"
		     "move%.l	%1,%%a1\n"
		     "1:\n"
		     "cpushl	%%dc,(%%a0)\n"
		     "lea	0x10(%%a0),%%a0\n"
		     "cmpa%.l	%%a0,%%a1\n"
		     "bne.b	1b\n"
		     "movec	%2,%%cacr\n"
		     : /* no return */
		     : "a" (__pa(vstart)), "a" (__pa(vend)),
		       "r" (shadow_cacr),
		       "i" (CF_CACR_SPA)
		     : "a0", "a1", "d0");
}

/**
 * cf_dcache_inv_range - Invalidate data cache in range
 * @vstart - starting virtual address
 * @vend: ending virtual address
 *
 * Invalidate the virtual range *vstart* -> *vend* using the enhanced
 * cpushl instruction to search by physical address.
 *
 * Assumes CACR[DDPI] is 0
 *
 * Sets CACR[SPA] and CACR[IVO] to 1 for the duration of this routine
 */
static inline void cf_dcache_inv_range(unsigned long vstart,
				       unsigned long vend)
{
	/* align on set boundary */
	vstart &= 0xfffffff0;
	vend = (vend + (CACHE_LINE_SIZE-1)) & 0xfffffff0;

	asm volatile("nop\n"
		     "move%.l   %2,%%d0\n"
		     "or%.l	%3,%%d0\n"
		     "movec	%%d0,%%cacr\n"
		     "move%.l	%0,%%a0\n"
		     "move%.l	%1,%%a1\n"
		     "1:\n"
		     "cpushl	%%dc,(%%a0)\n"
		     "lea	0x10(%%a0),%%a0\n"
		     "cmpa%.l	%%a0,%%a1\n"
		     "bne.b	1b\n"
		     "movec	%2,%%cacr\n"
		     : /* no return */
		     : "a" (__pa(vstart)), "a" (__pa(vend)),
		       "r" (shadow_cacr),
		       "i" (CF_CACR_SPA | CF_CACR_IVO)
		     : "a0", "a1", "d0");
}

/**
 * cf_dcache_push_range - Push data cache in range
 * @vstart - starting virtual address
 * @vend: ending virtual address
 *
 * Push the virtual range *vstart* -> *vend* using the enhanced
 * cpushl instruction to search by physical address.
 *
 * Assumes CACR[IVO] is 0
 *
 * Sets CACR[SPA] and CACR[DDPI] to 1 for the duration of this routine
 */
static inline void cf_dcache_push_range(unsigned long vstart,
				       unsigned long vend)
{
	/* align on set boundary */
	vstart &= 0xfffffff0;
	vend = (vend + (CACHE_LINE_SIZE-1)) & 0xfffffff0;

	asm volatile("nop\n"
		     "move%.l   %2,%%d0\n"
		     "or%.l	%3,%%d0\n"
		     "movec	%%d0,%%cacr\n"
		     "move%.l	%0,%%a0\n"
		     "move%.l	%1,%%a1\n"
		     "1:\n"
		     "cpushl	%%dc,(%%a0)\n"
		     "lea	0x10(%%a0),%%a0\n"
		     "cmpa%.l	%%a0,%%a1\n"
		     "bne.b	1b\n"
		     "movec	%2,%%cacr\n"
		     : /* no return */
		     : "a" (__pa(vstart)), "a" (__pa(vend)),
		       "r" (shadow_cacr),
		       "i" (CF_CACR_SPA | CF_CACR_DPI)
		     : "a0", "a1", "d0");
}

/**
 * cf_icache_flush_range - Push dirty inst cache in range out and invalidate
 * @vstart - starting virtual address
 * @vend: ending virtual address
 *
 * Push the any dirty instr lines starting at paddr for len bytes and
 * invalidate those lines.  This should just be an invalidate since you
 * shouldn't be able to have dirty instruction cache.
 */
static inline void cf_icache_flush_range(unsigned long vstart, unsigned long vend)
{
	/* align on set boundary */
	vstart &= 0xfffffff0;
	vend = (vend + (CACHE_LINE_SIZE-1)) & 0xfffffff0;

	asm volatile("nop\n"
		     "move%.l   %2,%%d0\n"
		     "or%.l	%3,%%d0\n"
		     "movec	%%d0,%%cacr\n"
		     "move%.l	%0,%%a0\n"
		     "move%.l	%1,%%a1\n"
		     "1:\n"
		     "cpushl	%%ic,(%%a0)\n"
		     "lea	0x10(%%a0),%%a0\n"
		     "cmpa%.l	%%a0,%%a1\n"
		     "bne.b	1b\n"
		     "movec	%2,%%cacr\n"
		     : /* no return */
		     : "a" (__pa(vstart)), "a" (__pa(vend)),
		       "r" (shadow_cacr),
		       "i" (CF_CACR_SPA)
		     : "a0", "a1", "d0");
}

/**
 * flush_cache_mm - Flush an mm_struct
 * @mm: mm_struct to flush
 */
static inline void flush_cache_mm(struct mm_struct *mm)
{
	if (mm == current->mm)
		flush_bcache();
}

#define flush_cache_dup_mm(mm)	flush_cache_mm(mm)

/**
 * flush_cache_range - Flush a cache range
 * @vma: vma struct
 * @start: Starting address
 * @end: Ending address
 *
 * flush_cache_range must be a macro to avoid a dependency on
 * linux/mm.h which includes this file.
 */
static inline void flush_cache_range(struct vm_area_struct *vma,
	unsigned long start, unsigned long end)
{
	if (vma->vm_mm == current->mm)
		cf_cache_flush_range(start, end);
}

/**
 * flush_cache_page - Flush a page of the cache
 * @vma: vma struct
 * @vmaddr:
 * @pfn: page numer
 *
 * flush_cache_page must be a macro to avoid a dependency on
 * linux/mm.h which includes this file.
 */
static inline void flush_cache_page(struct vm_area_struct *vma,
	unsigned long vmaddr, unsigned long pfn)
{
	if (vma->vm_mm == current->mm)
		cf_cache_flush_range(vmaddr, vmaddr+PAGE_SIZE);
}

/**
 * __flush_page_to_ram - Push a page out of the cache
 * @vaddr: Virtual address at start of page
 *
 * Push the page at kernel virtual address *vaddr* and clear
 * the icache.
 */
static inline void __flush_page_to_ram(void *vaddr)
{
	asm volatile("nop\n"
		     "move%.l   %2,%%d0\n"
		     "or%.l	%3,%%d0\n"
		     "movec	%%d0,%%cacr\n"
		     "move%.l	%0,%%d0\n"
		     "and%.l	#0xfffffff0,%%d0\n"
		     "move%.l	%%d0,%%a0\n"
		     "move%.l	%1,%%d0\n"
		     "1:\n"
		     "cpushl	%%bc,(%%a0)\n"
		     "lea	0x10(%%a0),%%a0\n"
		     "subq%.l	#1,%%d0\n"
		     "bne.b	1b\n"
		     "movec	%2,%%cacr\n"
		     : : "a" (__pa(vaddr)), "i" (PAGE_SIZE / CACHE_LINE_SIZE),
			 "r" (shadow_cacr), "i" (CF_CACR_SPA)
		     : "a0", "d0");
}

/*
 * Various defines for the kernel.
 */

extern void cache_clear(unsigned long paddr, int len);
extern void cache_push(unsigned long paddr, int len);
extern void flush_icache_range(unsigned long address, unsigned long endaddr);

#define flush_cache_all()			flush_bcache()
#define flush_cache_vmap(start, end)		flush_bcache()
#define flush_cache_vunmap(start, end)		flush_bcache()

#define flush_dcache_range(vstart, vend)	cf_dcache_flush_range(vstart, vend)
#define flush_dcache_page(page)			__flush_page_to_ram(page_address(page))
#define flush_dcache_mmap_lock(mapping)		do { } while (0)
#define flush_dcache_mmap_unlock(mapping)	do { } while (0)

#define flush_icache_page(vma, page)		__flush_page_to_ram(page_address(page))

/**
 * copy_to_user_page - Copy memory to user page
 */
static inline void copy_to_user_page(struct vm_area_struct *vma,
				     struct page *page, unsigned long vaddr,
				     void *dst, void *src, int len)
{
	memcpy(dst, src, len);
	cf_cache_flush(page_to_phys(page), PAGE_SIZE);
}

/**
 * copy_from_user_page - Copy memory from user page
 */
static inline void copy_from_user_page(struct vm_area_struct *vma,
				       struct page *page, unsigned long vaddr,
				       void *dst, void *src, int len)
{
	cf_cache_flush(page_to_phys(page), PAGE_SIZE);
	memcpy(dst, src, len);
}

#endif /* M68K_CF_5445x_CACHEFLUSH_H */
