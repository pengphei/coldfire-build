#ifndef _M68K_SWAB_H
#define _M68K_SWAB_H

#include <asm/types.h>
#include <linux/compiler.h>

/*#define __SWAB_64_THRU_32__

#if defined (__mcfisaaplus__) || defined (__mcfisac__)
static inline __attribute_const__ __u32 __arch_swab32(__u32 val)
{
	__asm__("byterev %0" : "=d" (val) : "0" (val));
	return val;
}

#define __arch_swab32 __arch_swab32
#elif !defined(__uClinux__)

static inline __attribute_const__ __u32 __arch_swab32(__u32 val)
{
	__asm__("rolw #8,%0; swap %0; rolw #8,%0" : "=d" (val) : "0" (val));
	return val;
}
#define __arch_swab32 __arch_swab32
#endif
*/
#if defined(__GNUC__)
#if defined(__mcfisaaplus__) || defined(__mcfisac__)
static inline __attribute_const__ __u32 ___arch__swab32(__u32 val)
{
	__asm__ ("byterev %0" : "=d" (val) : "0" (val));
	return val;
}
#define __arch__swab32(x) ___arch__swab32(x)
#elif !defined(__mcoldfire__)
static inline __attribute_const__ __u32 ___arch__swab32(__u32 val)
{
	__asm__("rolw #8,%0; swap %0; rolw #8,%0" : "=d" (val) : "0" (val));
	return val;
}
#define __arch__swab32(x) ___arch__swab32(x)

#endif
#endif

#if defined(__GNUC__) && !defined(__STRICT_ANSI__) || defined(__KERNEL__)
#  define __BYTEORDER_HAS_U64__
#  define __SWAB_64_THRU_32__
#endif

#endif /* _M68K_SWAB_H */
