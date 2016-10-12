/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004-2007, 2010 Freescale Semiconductor, Inc.
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
#include <watchdog.h>
#include <asm/immap.h>
#include <asm/rtc.h>

#if defined(CONFIG_CMD_NET)
#include <config.h>
#include <net.h>
#include <asm/fec.h>
#endif

void init_fbcs(void)
{
	volatile fbcs_t *fbcs = (fbcs_t *) MMAP_FBCS;

#if !defined(CONFIG_SERIAL_BOOT)
#if (defined(CONFIG_SYS_CS0_BASE) && defined(CONFIG_SYS_CS0_MASK) && defined(CONFIG_SYS_CS0_CTRL))
	fbcs->csar0 = CONFIG_SYS_CS0_BASE;
	fbcs->cscr0 = CONFIG_SYS_CS0_CTRL;
	fbcs->csmr0 = CONFIG_SYS_CS0_MASK;
#endif
#endif

#if (defined(CONFIG_SYS_CS1_BASE) && defined(CONFIG_SYS_CS1_MASK) && defined(CONFIG_SYS_CS1_CTRL))
	fbcs->csar1 = CONFIG_SYS_CS1_BASE;
	fbcs->cscr1 = CONFIG_SYS_CS1_CTRL;
	fbcs->csmr1 = CONFIG_SYS_CS1_MASK;
#endif

#if (defined(CONFIG_SYS_CS2_BASE) && defined(CONFIG_SYS_CS2_MASK) && defined(CONFIG_SYS_CS2_CTRL))
	fbcs->csar2 = CONFIG_SYS_CS2_BASE;
	fbcs->cscr2 = CONFIG_SYS_CS2_CTRL;
	fbcs->csmr2 = CONFIG_SYS_CS2_MASK;
#endif

#if (defined(CONFIG_SYS_CS3_BASE) && defined(CONFIG_SYS_CS3_MASK) && defined(CONFIG_SYS_CS3_CTRL))
	fbcs->csar3 = CONFIG_SYS_CS3_BASE;
	fbcs->cscr3 = CONFIG_SYS_CS3_CTRL;
	fbcs->csmr3 = CONFIG_SYS_CS3_MASK;
#endif

#if (defined(CONFIG_SYS_CS4_BASE) && defined(CONFIG_SYS_CS4_MASK) && defined(CONFIG_SYS_CS4_CTRL))
	fbcs->csar4 = CONFIG_SYS_CS4_BASE;
	fbcs->cscr4 = CONFIG_SYS_CS4_CTRL;
	fbcs->csmr4 = CONFIG_SYS_CS4_MASK;
#endif

#if (defined(CONFIG_SYS_CS5_BASE) && defined(CONFIG_SYS_CS5_MASK) && defined(CONFIG_SYS_CS5_CTRL))
	fbcs->csar5 = CONFIG_SYS_CS5_BASE;
	fbcs->cscr5 = CONFIG_SYS_CS5_CTRL;
	fbcs->csmr5 = CONFIG_SYS_CS5_MASK;
#endif
}

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f(void)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

#ifdef CONFIG_MCF5441x
	volatile scm_t *scm = (scm_t *) MMAP_SCM;
	volatile pm_t *pm = (pm_t *) MMAP_PM;

	/* Disable core watchdog */
	scm->cwcr = 0;
	gpio->par_fbctl =
	    GPIO_PAR_FBCTL_ALE_FB_TS | GPIO_PAR_FBCTL_OE_FB_OE |
	    GPIO_PAR_FBCTL_FBCLK | GPIO_PAR_FBCTL_RW |
            GPIO_PAR_FBCTL_TA_TA;
	gpio->par_be =
	    GPIO_PAR_BE_BE3_BE3 | GPIO_PAR_BE_BE2_BE2 | GPIO_PAR_BE_BE1_BE1 |
	    GPIO_PAR_BE_BE0_BE0;


	pm->pmcr0 = 17;	/* eDMA */

	pm->pmcr0 = 18;	/* INTR0 */
	pm->pmcr0 = 19;	/* INTR1 */
	pm->pmcr0 = 20;	/* INTR2 */

	pm->pmcr0 = 22;	/* I2C0 */

	pm->pmcr0 = 28;	/* DTMR0 */
	pm->pmcr0 = 29;	/* DTMR1 */
	pm->pmcr0 = 30;	/* DTMR2 */
	pm->pmcr0 = 31;	/* DTMR3 */

	pm->pmcr0 = 32;	/* PIT0 */
	pm->pmcr0 = 33;	/* PIT1 */
	pm->pmcr0 = 34;	/* PIT2 */
	pm->pmcr0 = 35;	/* PIT3 */

	pm->pmcr0 = 36;	/* Edge Port */
	pm->pmcr0 = 37;	/* Edge Port */

	pm->pmcr0 = 44;	/* USB OTG */
	pm->pmcr0 = 45;	/* USB Host */

	pm->pmcr0 = 51;	/* esdhc */
	pm->pmcr0 = 53;	/* enet 0 */
	pm->pmcr0 = 54;	/* enet 1 */
//	pm->pmcr0 = 63;	/* nand */

#endif		/* CONFIG_MCF5441x */

#ifdef CONFIG_MCF5445x
	volatile scm1_t *scm1 = (scm1_t *) MMAP_SCM1;

	scm1->mpr = 0x77777777;
	scm1->pacra = 0;
	scm1->pacrb = 0;
	scm1->pacrc = 0;
	scm1->pacrd = 0;
	scm1->pacre = 0;
	scm1->pacrf = 0;
	scm1->pacrg = 0;

	/* FlexBus */
	gpio->par_be =
	    GPIO_PAR_BE_BE3_BE3 | GPIO_PAR_BE_BE2_BE2 | GPIO_PAR_BE_BE1_BE1 |
	    GPIO_PAR_BE_BE0_BE0;
	gpio->par_fbctl =
	    GPIO_PAR_FBCTL_OE | GPIO_PAR_FBCTL_TA_TA | GPIO_PAR_FBCTL_RW_RW |
	    GPIO_PAR_FBCTL_TS_TS;

#ifdef CONFIG_FSL_I2C
	gpio->par_feci2c = GPIO_PAR_FECI2C_SCL_SCL | GPIO_PAR_FECI2C_SDA_SDA;
#endif
#endif		/* CONFIG_MCF5445x */

	/* FlexBus Chipselect */
	init_fbcs();

	icache_enable();
	icache_enable();
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r(void)
{
#ifdef CONFIG_MCFRTC
	volatile rtc_t *rtc = (volatile rtc_t *)(CONFIG_SYS_MCFRTC_BASE);
	volatile rtcex_t *rtcex = (volatile rtcex_t *)&rtc->extended;

	rtcex->gocu = (CONFIG_SYS_RTC_OSCILLATOR >> 16) & 0xFFFF;
	rtcex->gocl = CONFIG_SYS_RTC_OSCILLATOR & 0xFFFF;
#endif
	return (0);
}

void uart_port_conf(int port)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;
#ifdef CONFIG_MCF5441x
	volatile pm_t *pm = (pm_t *) MMAP_PM;
#endif

	/* Setup Ports: */
	switch (port) {
#ifdef CONFIG_MCF5441x
	case 0:
		pm->pmcr0 = 24;	/* UART0 */
		gpio->par_uart0 &=
		    (GPIO_PAR_UART0_U0RXD_MASK | GPIO_PAR_UART0_U0TXD_MASK);
		gpio->par_uart0 |=
		    (GPIO_PAR_UART0_U0RXD_U0RXD | GPIO_PAR_UART0_U0TXD_U0TXD);
		break;
	case 1:
		pm->pmcr0 = 25;	/* UART1 */
		gpio->par_uart1 &=
		    (GPIO_PAR_UART1_U1RXD_MASK | GPIO_PAR_UART1_U1TXD_MASK);
		gpio->par_uart1 |=
		    (GPIO_PAR_UART1_U1RXD_U1RXD | GPIO_PAR_UART1_U1TXD_U1TXD);
		break;
	case 2:
		pm->pmcr0 = 26;	/* UART2 */
		gpio->par_uart2 &=
		    (GPIO_PAR_UART2_U2RXD_MASK | GPIO_PAR_UART2_U2TXD_MASK);
		gpio->par_uart2 |=
		    (GPIO_PAR_UART2_U2RXD_U2RXD | GPIO_PAR_UART2_U2TXD_U2TXD);
		break;
	case 3:
		pm->pmcr0 = 27;	/* UART3 */
		gpio->par_dspi0 &=
		    (GPIO_PAR_DSPI0_SIN_MASK | GPIO_PAR_DSPI0_SOUT_MASK);
		gpio->par_dspi0 |=
		    (GPIO_PAR_DSPI0_SIN_U3RXD | GPIO_PAR_DSPI0_SOUT_U3TXD);
		break;
	case 4:
		pm->pmcr1 = 24; /* UART4 */
		gpio->par_uart0 &=
		    (GPIO_PAR_UART0_U0CTS_MASK | GPIO_PAR_UART0_U0RTS_MASK);
		gpio->par_uart0 |=
		    (GPIO_PAR_UART0_U0CTS_U4TXD | GPIO_PAR_UART0_U0RTS_U4RXD);
		break;
	case 5:
		pm->pmcr1 = 25; /* UART5 */
		gpio->par_uart1 &=
		    (GPIO_PAR_UART1_U1CTS_MASK | GPIO_PAR_UART1_U1RTS_MASK);
		gpio->par_uart1 |=
		    (GPIO_PAR_UART1_U1CTS_U5TXD | GPIO_PAR_UART1_U1RTS_U5RXD);
		break;
	case 6:
		pm->pmcr1 = 26; /* UART6 */
		gpio->par_uart2 &=
		    (GPIO_PAR_UART2_U2CTS_MASK | GPIO_PAR_UART2_U2RTS_MASK);
		gpio->par_uart2 |=
		    (GPIO_PAR_UART2_U2CTS_U6TXD | GPIO_PAR_UART2_U2RTS_U6RXD);
		break;
	case 7:
		pm->pmcr1 = 27; /* UART7 */
		gpio->par_ssi0h &= (GPIO_PAR_SSI0H_RXD_MASK);
		gpio->par_ssi0l &= (GPIO_PAR_SSI0L_BCLK_MASK);
		gpio->par_ssi0h |= (GPIO_PAR_SSI0H_FS_U7TXD);
		gpio->par_ssi0l |= (GPIO_PAR_SSI0L_BCLK_U7RXD);
		break;
	case 8:
		pm->pmcr1 = 28; /* UART8 */
		gpio->par_cani2c &=
		    (GPIO_PAR_CANI2C_I2C0SCL_MASK |
		     GPIO_PAR_CANI2C_I2C0SDA_MASK);
		gpio->par_cani2c |=
		    (GPIO_PAR_CANI2C_I2C0SCL_U8TXD |
		     GPIO_PAR_CANI2C_I2C0SDA_U8RXD);
		break;
	case 9:
		pm->pmcr1 = 29; /* UART9 */
		gpio->par_cani2c &=
		    (GPIO_PAR_CANI2C_CAN1TX_MASK | GPIO_PAR_CANI2C_CAN1RX_MASK);
		gpio->par_cani2c |=
		    (GPIO_PAR_CANI2C_CAN1TX_U9TXD |
		     GPIO_PAR_CANI2C_CAN1RX_U9RXD);
		break;
#endif
#ifdef CONFIG_MCF5445x
	case 0:
		gpio->par_uart =
		    (GPIO_PAR_UART_U0TXD_U0TXD | GPIO_PAR_UART_U0RXD_U0RXD);
		break;
	case 1:
		gpio->par_uart =
		    (GPIO_PAR_UART_U1TXD_U1TXD | GPIO_PAR_UART_U1RXD_U1RXD);
		break;
#endif
	}
}

#if defined(CONFIG_CMD_NET)
int fecpin_setclear(struct eth_device *dev, int setclear)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	struct fec_info_s *info = (struct fec_info_s *)dev->priv;

#ifdef CONFIG_MCF5441x
	if (setclear) {
		gpio->par_fec = 0x03;
		gpio->srcr_fec = 0x0F;
		gpio->par_simp0h = ( gpio->par_simp0h &
			GPIO_PAR_SIMP0H_DAT_MASK) |
			GPIO_PAR_SIMP0H_DAT_GPIO;
		gpio->pddr_g = 	( gpio->pddr_g &
			GPIO_PDDR_G4_MASK) |
			GPIO_PDDR_G4_OUTPUT;
		gpio->podr_g &= GPIO_PODR_G4_MASK;

	} else {
		gpio->par_fec &= GPIO_PAR_FEC_FEC_MASK;
	}
#endif

#ifdef CONFIG_MCF5445x
	if (setclear) {
		gpio->par_feci2c |=
		    (GPIO_PAR_FECI2C_MDC0_MDC0 | GPIO_PAR_FECI2C_MDIO0_MDIO0);

		if (info->iobase == CONFIG_SYS_FEC0_IOBASE)
			gpio->par_fec |= GPIO_PAR_FEC_FEC0_RMII_GPIO;
		else
			gpio->par_fec |= GPIO_PAR_FEC_FEC1_RMII_ATA;
	} else {
		gpio->par_feci2c &=
		    ~(GPIO_PAR_FECI2C_MDC0_MDC0 | GPIO_PAR_FECI2C_MDIO0_MDIO0);

		if (info->iobase == CONFIG_SYS_FEC0_IOBASE)
			gpio->par_fec &= GPIO_PAR_FEC_FEC0_MASK;
		else
			gpio->par_fec &= GPIO_PAR_FEC_FEC1_MASK;
	}
#endif
	return 0;
}
#endif

#ifdef CONFIG_CF_DSPI
void cfspi_port_conf(void)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

#ifdef CONFIG_MCF5441x
	volatile pm_t *pm = (pm_t *) MMAP_PM;

	gpio->par_dspi0 = GPIO_PAR_DSPI0_SIN_DSPI0SIN | GPIO_PAR_DSPI0_SOUT_DSPI0SOUT |
	    GPIO_PAR_DSPI0_SCK_DSPI0SCK;
	gpio->srcr_dspiow = 3;

	pm->pmcr0 = 23;	/* DSPI0 */
#endif
#ifdef CONFIG_MCF5445x
	gpio->par_dspi = GPIO_PAR_DSPI_SIN_SIN | GPIO_PAR_DSPI_SOUT_SOUT |
	    GPIO_PAR_DSPI_SCK_SCK;
#endif
}

int cfspi_claim_bus(uint bus, uint cs)
{
	volatile dspi_t *dspi = (dspi_t *) MMAP_DSPI;
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	if ((dspi->sr & DSPI_SR_TXRXS) != DSPI_SR_TXRXS)
		return -1;

	/* Clear FIFO and resume transfer */
	dspi->mcr &= ~(DSPI_MCR_CTXF | DSPI_MCR_CRXF);

#ifdef CONFIG_MCF5441x
	switch (cs) {
	case 0:
		gpio->par_dspi0 &= GPIO_PAR_DSPI0_PCS0_MASK;
		gpio->par_dspi0 |= GPIO_PAR_DSPI0_PCS0_DSPI0PCS0;
		break;
	case 1:
		gpio->par_dspiow &= ~GPIO_PAR_DSPIOW_DSPI0PSC1;
		gpio->par_dspiow |= GPIO_PAR_DSPIOW_DSPI0PSC1;
		break;
	}
#endif

#ifdef CONFIG_MCF5445x
	switch (cs) {
	case 0:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS0_PCS0;
		gpio->par_dspi |= GPIO_PAR_DSPI_PCS0_PCS0;
		break;
	case 1:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS1_PCS1;
		gpio->par_dspi |= GPIO_PAR_DSPI_PCS1_PCS1;
		break;
	case 2:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS2_PCS2;
		gpio->par_dspi |= GPIO_PAR_DSPI_PCS2_PCS2;
		break;
	case 5:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS5_PCS5;
		gpio->par_dspi |= GPIO_PAR_DSPI_PCS5_PCS5;
		break;
	}
#endif
	return 0;
}

void cfspi_release_bus(uint bus, uint cs)
{
	volatile dspi_t *dspi = (dspi_t *) MMAP_DSPI;
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	dspi->mcr &= ~(DSPI_MCR_CTXF | DSPI_MCR_CRXF);	/* Clear FIFO */

#ifdef CONFIG_MCF5441x
	switch (cs) {
	case 0:
/*		gpio->par_dspi0 &= GPIO_PAR_DSPI0_PCS0_MASK;*/
		break;
	case 1:
		gpio->par_dspiow &= ~GPIO_PAR_DSPIOW_DSPI0PSC1;
		break;
	}
#endif

#ifdef CONFIG_MCF5445x
	switch (cs) {
	case 0:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS0_PCS0;
		break;
	case 1:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS1_PCS1;
		break;
	case 2:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS2_PCS2;
		break;
	case 5:
		gpio->par_dspi &= ~GPIO_PAR_DSPI_PCS5_PCS5;
		break;
	}
#endif
}
#endif
