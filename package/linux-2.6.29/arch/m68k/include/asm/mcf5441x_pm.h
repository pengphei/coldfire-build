/*
 * Copyright(C) 2010 Freescale Semiconductor, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef __MCF5441X_PM_H__
#define __MCF5441X_PM_H__

/*********************************************************************
*
* Power Management Module Register Definition
*
*********************************************************************/

#define MCF_PM_WCR	MCF_REG08(0xFC040013)
#define MCF_PM_PPMSR0	MCF_REG08(0xFC04002C)
#define MCF_PM_PPMCR0	MCF_REG08(0xFC04002D)
#define MCF_PM_PPMSR1	MCF_REG08(0xFC04002E)
#define MCF_PM_PPMCR1	MCF_REG08(0xFC04002F)
#define MCF_PM_PPMHR0	MCF_REG32(0xFC040030)
#define MCF_PM_PPMLR0	MCF_REG32(0xFC040034)
#define MCF_PM_PPMHR1	MCF_REG32(0xFC040038)
#define MCF_PM_PPMLR1	MCF_REG32(0xFC04003C)
#define MCF_PM_LPCR	MCF_REG08(0xEC090007)
#define MCF_PM_MISCCR	MCF_REG16(0xEC09000E)
#define MCF_PM_CDHR	MCF_REG16(0xEC090010)
#define MCF_PM_CDLR	MCF_REG16(0xEC090012)

#endif
