/*
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * Author: ChengJu Cai<B22600@freescale.com>
 *
 * Description:
 * This file is the register definition of m5445x's SCM module
 *
 * Changelog:
 * Thur Sep 03 2009 ChengJu Cai <B22600@freescale.com>
 * - create
 *
 * This file is part of the Linux kernel
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
#ifndef __MCF5445X_SCM_H__
#define __MCF5445X_SCM_H__

/*********************************************************************
*
* System Control Module (SCM)
*
*********************************************************************/

/* Core Watchdog(CW) Register read/write macros */
#define MCF_SCM_CWCR    MCF_REG16(0xFC040016)	/* CW Control Register */
#define MCF_SCM_CWSR    MCF_REG08(0xFC04001B)	/* CW Service Register */

/* CW Bit definitions and macros for SWCR */
#define MCF_SCM_CWCR_RO		(0x8000)		/* CWCR is read-only */
#define MCF_SCM_CWCR_CWRWR	(0x0100)		/* CW run when halted */
#define MCF_SCM_CWCR_CWE	(0x0080)		/* CW timer enable */
#define MCF_SCM_CWCR_CWRI(x)	(((x) & 0x03) << 5)	/* CW reset/interrupt */
#define MCF_SCM_CWCR_CWT(x)	(((x) & 0x1F) << 0)	/* CW time-out period */

#ifdef CONFIG_M54455
#define MCF_GPT_MAX_TIMEOUT	16
#elif defined(CONFIG_M54451)
#define MCF_GPT_MAX_TIMEOUT	17
#endif
/********************************************************************/

#endif /* __MCF5445X_SCM_H__ */
