/*
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

#include <common.h>

#if defined(CONFIG_CMD_DATE)

#include <command.h>
#include <rtc.h>
#include <asm/immap.h>
#include <asm/rrtc.h>

#ifndef CONFIG_SYS_MCFRRTC_BASE
#error RTC_BASE is not defined!
#endif

#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)
#define	STARTOFTIME		2112

void rtc_unlock(void)
{
	volatile rtc_t *rtc = (rtc_t *) (CONFIG_SYS_MCFRRTC_BASE);
	printf("unlock\n");
	rtc->cr |= 0;
	rtc->cr |= 1;
	rtc->cr |= 3;
	rtc->cr |= 2;
}

int rtc_get(struct rtc_time *tmp)
{
	volatile rtc_t *rtc = (rtc_t *) (CONFIG_SYS_MCFRRTC_BASE);
	char year;

	year = (rtc->yearmon >> 8);
	tmp->tm_year = year + STARTOFTIME;
	tmp->tm_mon = rtc->yearmon & 0xFF;
	tmp->tm_mday = rtc->days & 0xFF;
	tmp->tm_wday = rtc->days >> 8;
	tmp->tm_hour = rtc->hourmin >> 8;
	tmp->tm_min = rtc->hourmin & 0xFF;
	tmp->tm_sec = rtc->seconds;

#ifdef RTC_DEBUG
	printf("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	       tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	       tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif

	return 0;
}

int rtc_set(struct rtc_time *tmp)
{
	volatile rtc_t *rtc = (rtc_t *) (CONFIG_SYS_MCFRRTC_BASE);
	char year;
	int unlock = 0;

	if (tmp->tm_year > 2239) {
		printf("Unable to handle. Exceeding integer limitation!\n");
		tmp->tm_year = 2239;
	}

#ifdef RTC_DEBUG
	printf("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	       tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	       tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif

	/* Check to see if non-leap year's Feb set to 29 */
	if (!isleap(tmp->tm_year)) {
		if ((tmp->tm_mon == 2) && (tmp->tm_mday == 29)) {
			printf("Not a leap year!\n");
			return -1;
		}
	}

	/*
	 * The register are locked after 15 secs after power-on-reset.
	 * When the registers are unlocked, they remain unlocked for two secs
	 * then automatically locked
	 */
	if ((rtc->sr & RTC_SR_WPE) == RTC_SR_WPE) {
		rtc_unlock();
		unlock = 1;
	}

	year = tmp->tm_year - STARTOFTIME;
	rtc->yearmon = (year << 8) | tmp->tm_mon;
	rtc->days = (tmp->tm_wday << 8) | tmp->tm_mday;
	rtc->hourmin = (tmp->tm_hour << 8) | tmp->tm_min;
	rtc->seconds = tmp->tm_sec;

	if (unlock)
		rtc->cr |= 2;	/* locked it */

	return 0;
}

void rtc_reset(void)
{
	volatile rtc_t *rtc = (rtc_t *) (CONFIG_SYS_MCFRRTC_BASE);
	int unlock = 0;

	rtc->cr |= RTC_CR_SWR;

	if ((rtc->sr & RTC_SR_WPE) == RTC_SR_WPE) {
		rtc_unlock();
		unlock = 1;
	}

	if ((rtc->cfg_data & RTC_CFG_DATA_OSCEN) == 0) {
#ifdef CONFIG_RTC_DAYLIGHTSAVING
		rtc->dst_day = CONFIG_RTC_DST_DAY;
		rtc->dst_mon = CONFIG_RTC_DST_MON;
		rtc->dst_hour = CONFIG_RTC_DST_HOUR;
		rtc->cr |= RTC_CR_DSTEN;
#endif

		rtc->cfg_data |= RTC_CFG_DATA_OSCEN;
		printf("real-time-clock was stopped. Now starting...\n");
	}

	if (unlock)
		rtc->cr |= 2;
}

#endif				/* CONFIG_MCFRTC && CONFIG_CMD_DATE */
