#ifndef __ASMm68k_AUXVEC_H
#define __ASMm68k_AUXVEC_H
/*
 * Architecture-neutral AT_ values in 0-17, leave some room
 * for more of them.
 */

#ifdef CONFIG_VDSO
/* Entry point to the vdso page */
#define	AT_SYSINFO_EHDR		33
#endif

#endif
