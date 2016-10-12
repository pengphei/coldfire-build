/*
 * Linker script for vdso DSO.  The vdso page is an ELF shared
 * object prelinked to its virtual address, and with only one read-only
 * segment (that fits in one page).  This script controls its layout.
 *
 * Based on arch/sh/kernel/vsyscall/vsyscall.lds.S
 *
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Kurt Mahan <kmahan@freescale.com>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

OUTPUT_FORMAT("elf32-m68k", "elf32-m68k", "elf32-m68k")
OUTPUT_ARCH(m68k)

/* The ELF entry point can be used to set the AT_SYSINFO value.  */
ENTRY(__kernel_read_tp);

SECTIONS
{
	. = SIZEOF_HEADERS;

	.hash		: { *(.hash) }			:text
	.gnu.hash	: { *(.gnu.hash) }
	.dynsym		: { *(.dynsym) }
	.dynstr		: { *(.dynstr) }
	.gnu.version	: { *(.gnu.version) }
	.gnu.version_d	: { *(.gnu.version_d) }
	.gnu.version_r	: { *(.gnu.version_r) }

	/*
	 * This linker script is used both with -r and with -shared.
	 * For the layouts to match, we need to skip more than enough
	 * space for the dynamic symbol table et al.  If this amount
	 * is insufficient, ld -shared will barf.  Just increase it here.
	 */
	. = 0x400;

	.text		: { *(.text) } 			:text	=0x90909090
	.note		: { *(.note.*) }		:text	:note
	.eh_frame_hdr	: { *(.eh_frame_hdr ) }		:text	:eh_frame_hdr
	.eh_frame	: {
		KEEP (*(.eh_frame))
		LONG (0)
	}						:text
	.dynamic	: { *(.dynamic) }		:text	:dynamic
	.useless	: {
	      *(.got.plt) *(.got)
	      *(.data .data.* .gnu.linkonce.d.*)
	      *(.dynbss)
	      *(.bss .bss.* .gnu.linkonce.b.*)
	}						:text
}

/*
 * We must supply the ELF program headers explicitly to get just one
 * PT_LOAD segment, and set the flags explicitly to make segments read-only.
 */
PHDRS
{
	text		PT_LOAD FILEHDR PHDRS FLAGS(5);	/* PF_R|PF_X */
	dynamic		PT_DYNAMIC FLAGS(4);		/* PF_R */
	note		PT_NOTE FLAGS(4);		/* PF_R */
	eh_frame_hdr	PT_GNU_EH_FRAME;
}

/*
 * This controls what symbols we export from the DSO.
 */
VERSION
{
	LINUX_2.6 {
	global:
		__kernel_read_tp;
		__kernel_atomic_cmpxchg_32;
		__kernel_atomic_barrier;

	local: *;
	};
}
