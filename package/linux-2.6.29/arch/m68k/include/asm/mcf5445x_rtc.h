/*
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * Author: ChengJu Cai, B22600@freescale.com
 *
 * Description:
 * This file is the register definition of m5445x RTC module
 *
 * Changelog:
 *
 *
 * This file is part of the Linux kernel
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __MCF5445X_RTC_H__
#define __MCF5445X_RTC_H__

/*********************************************************************
*
* Real-time Clock (RTC)
*
*********************************************************************/
#ifndef MCFINT_VECBASE
#define MCFINT_VECBASE          64
#endif

#define MCFSIM_ICR_RTC		(0xFC048040 + 63)

/* Register read/write macros */
#define MCF_RTC_HOURMIN		0xFC03C000
#define MCF_RTC_SECONDS		0xFC03C004
#define MCF_RTC_ALRM_HM		0xFC03C008
#define MCF_RTC_ALRM_SEC	0xFC03C00C
#define MCF_RTC_CR		0xFC03C010
#define MCF_RTC_ISR		0xFC03C014
#define MCF_RTC_IER		0xFC03C018
#define MCF_RTC_STPWCH		0xFC03C01C
#define MCF_RTC_DAYS		0xFC03C020
#define MCF_RTC_ALRM_DAY	0xFC03C024
#define MCF_RTC_GOCU		0xFC03C034
#define MCF_RTC_GOCL		0xFC03C038

/* Bit definitions and macros for MCF_RTC_HOURMIN */
#define MCF_RTC_HOURMIN_MINUTES(x)	(((x) & 0x0000003F) << 0)
#define MCF_RTC_HOURMIN_HOURS(x)	(((x) & 0x0000001F) << 8)

/* Bit definitions and macros for MCF_RTC_SECONDS */
#define MCF_RTC_SECONDS_SECONDS(x)	(((x) & 0x0000003F) << 0)

/* Bit definitions and macros for MCF_RTC_ALRM_HM */
#define MCF_RTC_ALRM_HM_MINUTES(x)	(((x) & 0x0000003F) << 0)
#define MCF_RTC_ALRM_HM_HOURS(x)	(((x) & 0x0000001F) << 8)

/* Bit definitions and macros for MCF_RTC_ALRM_SEC */
#define MCF_RTC_ALRM_SEC_SECONDS(x)	(((x) & 0x0000003F) << 0)

/* Bit definitions and macros for MCF_RTC_CR */
#define MCF_RTC_CR_SWR		0x00000001
#define MCF_RTC_CR_EN		0x00000080

/* Bit definitions and macros for MCF_RTC_ISR */
#define MCF_RTC_ISR_SW		0x00000001
#define MCF_RTC_ISR_MIN		0x00000002
#define MCF_RTC_ISR_ALM		0x00000004
#define MCF_RTC_ISR_DAY		0x00000008
#define MCF_RTC_ISR_1HZ		0x00000010
#define MCF_RTC_ISR_HR		0x00000020
#define MCF_RTC_ISR_2HZ		0x00000080
#define MCF_RTC_ISR_SAM0	0x00000100
#define MCF_RTC_ISR_SAM1	0x00000200
#define MCF_RTC_ISR_SAM2	0x00000400
#define MCF_RTC_ISR_SAM3	0x00000800
#define MCF_RTC_ISR_SAM4	0x00001000
#define MCF_RTC_ISR_SAM5	0x00002000
#define MCF_RTC_ISR_SAM6	0x00004000
#define MCF_RTC_ISR_SAM7	0x00008000

/* Bit definitions and macros for MCF_RTC_IER */
#define MCF_RTC_IER_SW		0x00000001
#define MCF_RTC_IER_MIN		0x00000002
#define MCF_RTC_IER_ALM		0x00000004
#define MCF_RTC_IER_DAY		0x00000008
#define MCF_RTC_IER_1HZ		0x00000010
#define MCF_RTC_IER_HR		0x00000020
#define MCF_RTC_IER_2HZ		0x00000080
#define MCF_RTC_IER_SAM0	0x00000100
#define MCF_RTC_IER_SAM1	0x00000200
#define MCF_RTC_IER_SAM2	0x00000400
#define MCF_RTC_IER_SAM3	0x00000800
#define MCF_RTC_IER_SAM4	0x00001000
#define MCF_RTC_IER_SAM5	0x00002000
#define MCF_RTC_IER_SAM6	0x00004000
#define MCF_RTC_IER_SAM7	0x00008000

/* Bit definitions and macros for MCF_RTC_STPWCH */
#define MCF_RTC_STPWCH_CNT(x)	(((x)  &  0x0000003F) << 0)

/* Bit definitions and macros for MCF_RTC_DAYS */
#define MCF_RTC_DAYS_DAYS(x)	(((x)  &  0x0000FFFF) << 0)

/* Bit definitions and macros for MCF_RTC_ALRM_DAY */
#define MCF_RTC_ALRM_DAY_DAYS(x) (((x)  &  0x0000FFFF) << 0)

/* Interrupt source */
#define MCFINT_RTC		63
/*********************************************************************/

#endif
