/*
 * Setup vdso lib (.so) as binary image.
 */

#include <linux/init.h>

__INITDATA

	.globl vdso_bin_start, vdso_bin_end
vdso_bin_start:
	.incbin "arch/m68k/coldfire/vdso/vdso-lib.so"
vdso_bin_end:

__FINIT
