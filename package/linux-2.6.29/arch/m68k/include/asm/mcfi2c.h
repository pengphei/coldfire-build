/*
 *	mcfi2c.h -- ColdFire mcfv4/mcfv4e i2c controller support.
 *	Copyright 2009-2010 Freescale Semiconductor, Inc. All Rights Reserved.
 */
#ifndef MCF_I2C_H
#define MCF_I2C_H

/* Register read/write macros */
#if defined(CONFIG_M547X_8X)
#define MCF_I2AR     MCF_REG08(0x008F00)	/* I2C Address 		 */
#define MCF_I2FDR    MCF_REG08(0x008F04)	/* I2C Frequency Divider */
#define MCF_I2CR     MCF_REG08(0x008F08)	/* I2C Control		 */
#define MCF_I2SR     MCF_REG08(0x008F0C)	/* I2C Status		 */
#define MCF_I2DR     MCF_REG08(0x008F10)	/* I2C Data I/O		 */
#define MCF_I2ICR    MCF_REG08(0x008F20)	/* I2C Interrupt Control */
#elif defined(CONFIG_M5445X) || defined(CONFIG_M5441X)
#define MCF_I2AR     *(volatile u8 *)(0xFC058000)	/* I2C Address 		 */
#define MCF_I2FDR    *(volatile u8 *)(0xFC058004)	/* I2C Frequency Divider */
#define MCF_I2CR     *(volatile u8 *)(0xFC058008)	/* I2C Control		 */
#define MCF_I2SR     *(volatile u8 *)(0xFC05800C)	/* I2C Status		 */
#define MCF_I2DR     *(volatile u8 *)(0xFC058010)	/* I2C Data I/O		 */
#endif

/* Bit definitions and macros for MCF_I2C_I2AR */
#define MCF_I2AR_ADR(x)    (((x)&0x7F)<<1)

/* Bit definitions and macros for MCF_I2C_I2FDR */
#define MCF_I2FDR_IC(x)    (((x)&0x3F)<<0)

/* Bit definitions and macros for MCF_I2C_I2CR */
#define MCF_I2CR_RSTA      (0x04)
#define MCF_I2CR_TXAK      (0x08)
#define MCF_I2CR_MTX       (0x10)
#define MCF_I2CR_MSTA      (0x20)
#define MCF_I2CR_IIEN      (0x40)
#define MCF_I2CR_IEN       (0x80)

/* Bit definitions and macros for MCF_I2C_I2SR */
#define MCF_I2SR_RXAK      (0x01)
#define MCF_I2SR_IIF       (0x02)
#define MCF_I2SR_SRW       (0x04)
#define MCF_I2SR_IAL       (0x10)
#define MCF_I2SR_IBB       (0x20)
#define MCF_I2SR_IAAS      (0x40)
#define MCF_I2SR_ICF       (0x80)

/* Bit definitions and macros for MCF_I2C_I2ICR */
#if defined(CONFIG_M547X_8X)
#define MCF_I2ICR_IE       (0x01)
#define MCF_I2ICR_RE       (0x02)
#define MCF_I2ICR_TE       (0x04)
#define MCF_I2ICR_BNBE     (0x08)
#endif

/********************************************************************/
#endif
