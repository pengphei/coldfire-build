#ifndef __MMU_H
#define __MMU_H

#ifdef CONFIG_VDSO
typedef struct {
	unsigned long		id;
	void			*vdso;
} mm_context_t;
#else
/* Default "unsigned long" context */
typedef unsigned long mm_context_t;
#endif

#endif
