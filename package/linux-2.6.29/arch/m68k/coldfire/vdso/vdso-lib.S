/*
 * VDSO userspace code
 *
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Kurt Mahan <kmahan@freescale.com>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/linkage.h>
#include <asm/unistd.h>

	.text
/*
 * Read the thread pointer into A0 (and D0, for compatibility).
 */
ENTRY(__kernel_read_tp)
	.cfi_startproc
	movel	#__NR_read_tp,%d0
	trap	#0
	movel	%d0,%a0
	rts
	.cfi_endproc
	.size __kernel_read_tp,.-__kernel_read_tp

/*
 * Atomic compare exchange.  Can not clobber any registers
 * other than conditional codes.
 */
ENTRY(__kernel_atomic_cmpxchg_32)
	.cfi_startproc
	movel	%d2,%sp@-
	.cfi_adjust_cfa_offset 4
	.cfi_rel_offset %d2,0
	movel	%d0,%d2
	movel	#__NR_atomic_cmpxchg_32,%d0
	trap	#0
	movel	%sp@+,%d2
	.cfi_adjust_cfa_offset -4
	.cfi_restore d2
	rts
	.cfi_endproc
	.size __kernel_atomic_cmpxchg_32,.-__kernel_atomic_cmpxchg_32

/*
 * Atomic memory barrier.  Can not clobber any registers
 * other than condition codes.
 */
ENTRY(__kernel_atomic_barrier)
	.cfi_startproc
	/* no code needed for uniprocs */
	rts
	.cfi_endproc
	.size __kernel_atomic_barrier,.-__kernel_atomic_barrier

	.previous
