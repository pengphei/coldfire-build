/*
 * Copyright(C) 2010 Freescale Semiconductor, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef __MCF5441X_CCM_H__
#define __MCF5441X_CCM_H__

/*********************************************************************
*
* Chip Configuration Module (CCM)
*
*********************************************************************/

/* Register read/write macros */
#define MCF_CCM_CCR	MCF_REG16(0xEC090004)	/* Chip Configuration Register (Read-only) (256 or 360 TEPBGA) */
#define MCF_CCM_RCON	MCF_REG16(0xEC090008)	/* Reset Configuration (Read-only) (256 or 360 TEPBGA) */
#define MCF_CCM_CIR	MCF_REG16(0xEC09000A)	/* Chip Identification Register (Read-only) */
#define MCF_CCM_MISCCR	MCF_REG16(0xEC09000E)	/* Miscellaneous Control Register */
#define MCF_CCM_CDRH	MCF_REG16(0xEC090010)	/* Clock Divider Register */
#define MCF_CCM_CDRL	MCF_REG16(0xEC090012)	/* Clock Divider Register */
#define MCF_CCM_UOCSR	MCF_REG16(0xEC090014)	/* USB On-the-Go Controller Status Register */
#define MCF_CCM_UHCSR	MCF_REG16(0xEC090016)	/* USB On-the-Go Controller Status Register */
#define MCF_CCM_MISCCR3	MCF_REG16(0xEC090018)	/* Miscellaneous Control Register */
#define MCF_CCM_MISCCR2	MCF_REG16(0xEC09001A)	/* Miscellaneous Control Register */
#define MCF_CCM_ADCTSR	MCF_REG16(0xEC09001C)
#define MCF_CCM_DACTSR	MCF_REG16(0xEC09001E)
#define MCF_CCM_SBFSR	MCF_REG16(0xEC090020)	/* Serial Boot Facility Status Register (Read Only) */
#define MCF_CCM_SBFCR	MCF_REG16(0xEC090022)	/* Serial Boot Facility Control Register */
#define MCF_CCM_FNACR	MCF_REG16(0xEC090024)

/* Bit definitions and macros for CIR */
#define MCF_CCM_CIR_PRN(x)		(((x) & 0x003F))	/* Part revision number */
#define MCF_CCM_CIR_PIN(x)		(((x) & 0x03FF) << 6)	/* Part identification number */
#define MCF_CCM_CIR_PIN_MASK		(0xFFC0)
#define MCF_CCM_CIR_PRN_MASK		(0x003F)
#define MCF_CCM_CIR_PIN_MCF54410	(0x9F<<6)
#define MCF_CCM_CIR_PIN_MCF54415	(0xA0<<6)
#define MCF_CCM_CIR_PIN_MCF54416	(0xA1<<6)
#define MCF_CCM_CIR_PIN_MCF54417	(0xA2<<6)
#define MCF_CCM_CIR_PIN_MCF54418	(0xA3<<6)

/* Bit definitions and macros for MISCCR */
#define MCF_CCM_CIR_PRN(x)		(((x) & 0x003F))	/* Part revision number */
#define MCF_CCM_CIR_PIN(x)		(((x) & 0x03FF) << 6)	/* Part identification number */
#define MCF_CCM_CIR_PIN_MASK		(0xFFC0)
#define MCF_CCM_CIR_PRN_MASK		(0x003F)
#define MCF_CCM_CIR_PIN_MCF54410	(0x9F<<6)
#define MCF_CCM_CIR_PIN_MCF54415	(0xA0<<6)
#define MCF_CCM_CIR_PIN_MCF54416	(0xA1<<6)
#define MCF_CCM_CIR_PIN_MCF54417	(0xA2<<6)
#define MCF_CCM_CIR_PIN_MCF54418	(0xA3<<6)
#define MCF_CCM_MISCCR_PWM_EXTCLK(x)	(((x)&(0x0003)<<14)
#define MCF_CCM_MISCCR_PWM_EXTCLK_MASK	(0x3FFF)
#define MCF_CCM_MISCCR_PWM_EXTCLK_TMR0	(0x0000)
#define MCF_CCM_MISCCR_PWM_EXTCLK_TMR1	(0x4000)
#define MCF_CCM_MISCCR_PWM_EXTCLK_TMR2	(0x8000)
#define MCF_CCM_MISCCR_PWM_EXTCLK_TMR3	(0xC000)
#define MCF_CCM_MISCCR_LIMP		(0x1000)
#define MCF_CCM_MISCCR_BME		(0x0800)
#define MCF_CCM_MISCCR_BMT(x)		(((x)&0x0007)<<8)
#define MCF_CCM_MISCCR_BMT_MASK		(0xF8FF)
#define MCF_CCM_MISCCR_BMT_65536	(0)
#define MCF_CCM_MISCCR_BMT_32768	(1)
#define MCF_CCM_MISCCR_BMT_16384	(2)
#define MCF_CCM_MISCCR_BMT_8192		(3)
#define MCF_CCM_MISCCR_BMT_4096		(4)
#define MCF_CCM_MISCCR_BMT_2048		(5)
#define MCF_CCM_MISCCR_BMT_1024		(6)
#define MCF_CCM_MISCCR_BMT_512		(7)
#define MCF_CCM_MISCCR_SDHCSRC		(0x0040)
#define MCF_CCM_MISCCR_SSI1SRC		(0x0020)
#define MCF_CCM_MISCCR_SSI0SRC		(0x0010)
#define MCF_CCM_MISCCR_USBHOC		(0x0008)
#define MCF_CCM_MISCCR_USBOOC		(0x0004)
#define MCF_CCM_MISCCR_USBPUE		(0x0002)
#define MCF_CCM_MISCCR_USBSRC		(0x0001)
#define MCF_CCM_CDRH_SSI0DIV(x)		(((x)&0x00FF)<<8)
#define MCF_CCM_CDRH_SSI0DIV_MASK	(0x00FF)
#define MCF_CCM_CDRH_SSI1DIV(x)		(((x)&0x00FF))
#define MCF_CCM_CDRH_SSI1DIV_MASK	(0xFF00)
#define MCF_CCM_CDRL_LPDIV(x)		(((x)&0x000F)<<8)
#define MCF_CCM_CDRL_LPDIV_MASK		(0xFF0F)
#define MCF_CCM_CDR_LPDIV(x)		CCM_CDRL_LPDIV(x)

#define MCF_CCM_UOCSR_DPPD		(0x2000)
#define MCF_CCM_UOCSR_DMPD		(0x1000)
#define MCF_CCM_UOCSR_DRV_VBUS		(0x0800)
#define MCF_CCM_UOCSR_CRG_VBUS		(0x0400)
#define MCF_CCM_UOCSR_DCR_VBUS		(0x0200)
#define MCF_CCM_UOCSR_DPPU		(0x0100)
#define MCF_CCM_UOCSR_AVLD		(0x0080)
#define MCF_CCM_UOCSR_BVLD		(0x0040)
#define MCF_CCM_UOCSR_VVLD		(0x0020)
#define MCF_CCM_UOCSR_SEND		(0x0010)
#define MCF_CCM_UOCSR_PWRFLT		(0x0008)
#define MCF_CCM_UOCSR_WKUP		(0x0004)
#define MCF_CCM_UOCSR_UOMIE		(0x0002)
#define MCF_CCM_UOCSR_XPDE		(0x0001)

#define MCF_CCM_UHCSR_DRV_VBUS		(0x0010)
#define MCF_CCM_UHCSR_PWRFLT		(0x0008)
#define MCF_CCM_UHCSR_WKUP		(0x0004)
#define MCF_CCM_UHCSR_UOMIE		(0x0002)
#define MCF_CCM_UHCSR_XPDE		(0x0001)

#define MCF_CCM_MISCCR3_TMR_ENET	(0x1000)
#define MCF_CCM_MISCCR3_ENETCLK(x)	((((x)&7))<<8)
#define MCF_CCM_MISCCR3_ENETCLK_MASK	(0xF8FF)
#define MCF_CCM_MISCCR3_ENETCLK_MII	(0x0700)
#define MCF_CCM_MISCCR3_ENETCLK_OSC	(0x0600)
#define MCF_CCM_MISCCR3_ENETCLK_USB	(0x0500)
#define MCF_CCM_MISCCR3_ENETCLK_TMR3	(0x0400)
#define MCF_CCM_MISCCR3_ENETCLK_TMR2	(0x0300)
#define MCF_CCM_MISCCR3_ENETCLK_TMR1	(0x0200)
#define MCF_CCM_MISCCR3_ENETCLK_TMR0	(0x0100)
#define MCF_CCM_MISCCR3_ENETCLK_INTBUS	(0x0000)

#define MCF_CCM_MISCCR2_EXTCLKBYP	(0x0800)
#define MCF_CCM_MISCCR2_DDR2CLK		(0x0400)
#define MCF_CCM_MISCCR2_RGPIO_HALF	(0x0200)
#define MCF_CCM_MISCCR2_SWTSCR		(0x0100)
#define MCF_CCM_MISCCR2_PLLMODE(x)	(((x)&7)<<8)
#define MCF_CCM_MISCCR2_PLLMODE_MASK	(0xF8FF)
#define MCF_CCM_MISCCR2_DCCBYP		(0x0080)
#define MCF_CCM_MISCCR2_DAC1SEL		(0x0040)
#define MCF_CCM_MISCCR2_DAC0SEL		(0x0020)
#define MCF_CCM_MISCCR2_ADCEN		(0x0010)
#define MCF_CCM_MISCCR2_ADC7SEL		(0x0008)
#define MCF_CCM_MISCCR2_ADC3SEL		(0x0004)
#define MCF_CCM_MISCCR2_FBHALF		(0x0002)
#define MCF_CCM_MISCCR2_ULPI		(0x0001)

#define MCF_CCM_FNACR_PCR(x)		(((x)&0x0F)<<24)
#define MCF_CCM_FNACR_PCR_MASK		(0xF0FFFFFF)
#define MCF_CCM_FNACR_MCC(x)		((x)&0xFFFF)
#define MCF_CCM_FNACR_MCC_MASK		(0xFFFF0000)

#endif /* __MCF5445X_CCM_H__ */
