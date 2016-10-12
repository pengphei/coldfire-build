/*
 * Copyright (C) 2010 Freescale Semiconductor, Inc. All rights reserved.
 *
 * Lanttor.Guo@freescale.com
 *
 * Description:
 * This file is the register definition of m5441x RTC module
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __MCF5441X_RTC_H__
#define __MCF5441X_RTC_H__

/*********************************************************************
*
* Real-time Clock (RTC)
*
*********************************************************************/
#ifndef MCFINT_VECBASE
#define MCFINT_VECBASE          64
#endif

#define MCFSIM_ICR_RTC		(0xFC050040 + 26)

/* Register read/write macros */
#define MCF_RTC_YEARMON		0xFC0A8000
#define MCF_RTC_DAYS		0xFC0A8002
#define MCF_RTC_HOURMIN		0xFC0A8004
#define MCF_RTC_SECONDS		0xFC0A8006
#define MCF_RTC_ALRM_YRMON	0xFC0A8008
#define MCF_RTC_ALRM_DAYS	0xFC0A800A
#define MCF_RTC_ALRM_HM		0xFC0A800C
#define MCF_RTC_ALRM_SEC	0xFC0A800E
#define MCF_RTC_CR		0xFC0A8010
#define MCF_RTC_SR		0xFC0A8012
#define MCF_RTC_ISR		0xFC0A8014
#define MCF_RTC_IER		0xFC0A8016
#define MCF_RTC_COUNT_DN	0xFC0A8018
#define MCF_RTC_CFG_DATA	0xFC0A8020
#define MCF_RTC_DST_HOUR	0xFC0A8022
#define MCF_RTC_DST_MON		0xFC0A8024
#define MCF_RTC_DST_DAY		0xFC0A8026
#define MCF_RTC_COMPEN		0xFC0A8028
#define MCF_RTC_UP_CNTRH	0xFC0A8032
#define MCF_RTC_UP_CNTRL	0xFC0A8034

#define MCF_RTC_STANDBY_RAM_START	0xFC0A8040
#define MCF_RTC_STANDBY_RAM_END		0xFC0A8040

/* Bit definitions and macros for MCF_RTC_YEARMON */

#define MCF_RTC_YEARMON_YEAR_RD(x)		(((x) & 0xFF00) >> 8)
#define MCF_RTC_YEARMON_MON_RD(x)		((x) & 0x00FF)
#define MCF_RTC_YEARMON_YEAR_SET(x)		(((x) & 0x00FF) << 8)
#define MCF_RTC_YEARMON_MON_SET(x)		((x) & 0x000F)

/* Bit definitions and macros for MCF_RTC_DAYS */
#define MCF_RTC_DAYS_DAYWEEK_RD(x)		(((x) & 0xFF00) >> 8)
#define MCF_RTC_DAYS_DAY_RD(x)			((x) & 0x00FF)
#define MCF_RTC_DAYS_DAYWEEK_SET(x)		(((x) & 0x0007) << 8)
#define MCF_RTC_DAYS_DAY_SET(x)			((x) & 0x001F)

/* Bit definitions and macros for MCF_RTC_HOURMIN */
#define MCF_RTC_HOURMIN_HOURS_RD(x)		(((x) & 0xFF00) >> 8)
#define MCF_RTC_HOURMIN_MINUTES_RD(x)		((x) & 0x00FF)
#define MCF_RTC_HOURMIN_HOURS_SET(x)		(((x) & 0x001F) << 8)
#define MCF_RTC_HOURMIN_MINUTES_SET(x)		((x) & 0x003F)

/* Bit definitions and macros for MCF_RTC_SECONDS */
#define MCF_RTC_SECONDS_SECONDS_RD(x)		((x) & 0x00FF)
#define MCF_RTC_SECONDS_SECONDS_SET(x)		(((x) & 0x003F) << 0)

/* Bit definitions and macros for MCF_RTC_CR */
#define MCF_RTC_CR_WP		0x0002

/* Bit definitions and macros for MCF_RTC_ISR */
#define MCF_RTC_ISR_STW		0x0002
#define MCF_RTC_ISR_ALM		0x0004
#define MCF_RTC_ISR_DAY		0x0008
#define MCF_RTC_ISR_HR		0x0010
#define MCF_RTC_ISR_MIN		0x0020
#define MCF_RTC_ISR_1HZ		0x0040
#define MCF_RTC_ISR_2HZ		0x0080
#define MCF_RTC_ISR_SAM0	0x0100
#define MCF_RTC_ISR_SAM1	0x0200
#define MCF_RTC_ISR_SAM2	0x0400
#define MCF_RTC_ISR_SAM3	0x0800
#define MCF_RTC_ISR_SAM4	0x1000
#define MCF_RTC_ISR_SAM5	0x2000
#define MCF_RTC_ISR_SAM6	0x4000
#define MCF_RTC_ISR_SAM7	0x8000

/* Bit definitions and macros for MCF_RTC_IER */
#define MCF_RTC_IER_STW		0x0002
#define MCF_RTC_IER_ALM		0x0004
#define MCF_RTC_IER_DAY		0x0008
#define MCF_RTC_IER_HR		0x0010
#define MCF_RTC_IER_MIN		0x0020
#define MCF_RTC_IER_1HZ		0x0040
#define MCF_RTC_IER_2HZ		0x0080
#define MCF_RTC_IER_SAM0	0x0100
#define MCF_RTC_IER_SAM1	0x0200
#define MCF_RTC_IER_SAM2	0x0400
#define MCF_RTC_IER_SAM3	0x0800
#define MCF_RTC_IER_SAM4	0x1000
#define MCF_RTC_IER_SAM5	0x2000
#define MCF_RTC_IER_SAM6	0x4000
#define MCF_RTC_IER_SAM7	0x8000

/* Bit definitions and macros for MCF_RTC_CFG_DATA */
#define MCF_RTC_CFG_DATA_OSCBYP	0x0010
#define MCF_RTC_CFG_DATA_OSCEN	0x0008

/* Bit definitions and macros for MCF_RTC_SR */
#define MCF_RTC_SR_INVALID 	0x0001
#define MCF_RTC_SR_WPE		0x0010

/* Interrupt source */
#define MCFINT_RTC		(64+64+26)
/*********************************************************************/

#endif
