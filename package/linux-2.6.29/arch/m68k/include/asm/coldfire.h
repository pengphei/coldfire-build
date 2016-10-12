/****************************************************************************/

/*
 *	coldfire.h -- Motorola ColdFire CPU sepecific defines
 *
 *	(C) Copyright 1999-2006, Greg Ungerer (gerg@snapgear.com)
 *	(C) Copyright 2000, Lineo (www.lineo.com)
 *
 *	Shrek Wu b16972@freescale.com
 *	Copyright Freescale Semiconductor, Inc. 2009-2010
 */

/****************************************************************************/
#ifndef	coldfire_h
#define	coldfire_h
/****************************************************************************/


/*
 *	Define master clock frequency. This is essentially done at config
 *	time now. No point enumerating dozens of possible clock options
 *	here. Also the peripheral clock (bus clock) divide ratio is set
 *	at config time too.
 */
/*FIXME Jason*/
#if 0
#ifdef CONFIG_CLOCK_SET
#define	MCF_CLK		CONFIG_CLOCK_FREQ
#define	MCF_BUSCLK	(CONFIG_CLOCK_FREQ / CONFIG_CLOCK_DIV)
#else
#error "Don't know what your ColdFire CPU clock frequency is??"
#endif
#endif 


#define MCF_CLK     	CONFIG_MCFCLK
#define MCF_BUSCLK	(CONFIG_MCFCLK/2)


#if defined(CONFIG_M520x)
#define	MCF_IPSBAR	0xFC000000
#else
#define	MCF_IPSBAR	0x40000000
#endif

#if defined(CONFIG_M5445X) || defined(CONFIG_M5441X)
#define MCF_MBAR	0x0
/*
 * Even though RAMBAR1 macro should be in the 0x8xxxxxxx range,
 * here set the CONFIG_SDRAM_BASE value to it to use
 * SDRAM memory, not SRAM memory.
 */
#define MCF_RAMBAR1	(CONFIG_SDRAM_BASE)
#elif defined(CONFIG_M547X_8X)
#define MCF_MBAR        0xF0000000
#define MCF_MMUBAR      0xF1000000
#define MCF_RAMBAR0     0xF3000000
#define MCF_RAMBAR1     0xF3001000
#else
/*
 *	Define the processor support peripherals base address.
 *	This is generally setup by the boards start up code.
 */
#define	MCF_MBAR	0x10000000
#define	MCF_MBAR2	0x80000000
#endif

#ifdef __ASSEMBLY__
#define REG32
#define REG16
#define REG08
#else  /* __ASSEMBLY__ */
#define REG32(x) ((volatile unsigned long  *)(x))
#define REG16(x) ((volatile unsigned short *)(x))
#define REG08(x) ((volatile unsigned char  *)(x))

#define MCF_REG32(x) *(volatile unsigned long  *)(MCF_MBAR+(x))
#define MCF_REG16(x) *(volatile unsigned short *)(MCF_MBAR+(x))
#define MCF_REG08(x) *(volatile unsigned char  *)(MCF_MBAR+(x))

void cacr_set(unsigned long);
unsigned long cacr_get(void);

#define coldfire_enable_irq0(irq)	MCF_INTC0_CIMR = (irq);

#define coldfire_enable_irq1(irq)	MCF_INTC1_CIMR = (irq);

#define coldfire_disable_irq0(irq)	MCF_INTC0_SIMR = (irq);

#define coldfire_disable_irq1(irq)	MCF_INTC1_SIMR = (irq);

#define getiprh()			MCF_INTC0_IPRH

#endif /* __ASSEMBLY__ */


#if defined(CONFIG_M523x) || defined(CONFIG_M527x) || defined(CONFIG_M528x) || \
    defined(CONFIG_M520x)
#undef MCF_MBAR
#define	MCF_MBAR	MCF_IPSBAR
#elif defined(CONFIG_M532x)
#undef MCF_MBAR
#define MCF_MBAR	0x00000000
#endif

/****************************************************************************/
#endif	/* coldfire_h */
