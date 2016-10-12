/*
 * Robust RealTime Clock
 *
 * Copyright (C) 2010 Freescale Semiconductor, Inc. All rights reserved.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __MCFRTC_H__
#define __MCFRTC_H__

/* Real time Clock */
typedef struct rtc_ctrl {
	u16 yearmon; /*00*/
	u16 days;
	u16 hourmin;
	u16 seconds;
	u16 alrm_ym; /*0x08*/
	u16 alrm_day;
	u16 alrm_hourmin;
	u16 alrm_sec;
	u16 cr; /*10*/
	u16 sr;
	u16 isr;
	u16 ier;
	u16 cnt_dn; /*0x18*/
	u32 res1;
	u16 res2;
	u16 cfg_data; /*0x20*/
	u16 dst_hour;
	u16 dst_mon;
	u16 dst_day;
	u16 compen; /*28*/
	u16 res3;
	u16 cntrh;
	u16 cntrl;
} rtc_t;

/* SR */
#define RTC_SR_WPE		0x0010

#define RTC_CFG_DATA_OSCEN	0x0008
#define RTC_CR_SWR		0x0100

#endif				/* __MCFRTC_H__ */
