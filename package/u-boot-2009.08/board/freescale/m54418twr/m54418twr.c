/*
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
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
#include <spi.h>
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	/*
	 * need to to:
	 * Check serial flash size. if 2mb evb, else 8mb demo
	 */
	puts("Board: ");
	puts("Freescale M54418 Tower System\n");
	return 0;
};

phys_size_t initdram(int board_type)
{
	u32 dramsize;

#if defined(CONFIG_SERIAL_BOOT)
	/*
	 * Serial Boot: The dram is already initialized in start.S
	 * only require to return DRAM size
	 */
	dramsize = CONFIG_SYS_SDRAM_SIZE * 0x100000;
#else
	volatile sdramc_t *sdram = (volatile sdramc_t *)(MMAP_SDRAM);
	volatile ccm_t *ccm = (volatile ccm_t *)MMAP_CCM;
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	volatile pm_t *pm = (pm_t *) MMAP_PM;
	u32 i;

	dramsize = CONFIG_SYS_SDRAM_SIZE * 0x100000;

	for (i = 0x13; i < 0x20; i++) {
		if (dramsize == (1 << i))
			break;
	}

	pm->pmcr0 = 0x2E;
	gpio->mscr_sdram = 1;
	ccm->misccr2 &= ~CCM_MISCCR2_FBHALF;
	ccm->misccr2 |= CCM_MISCCR2_DDR2CLK;

	sdram->rcrcr = 0x40000000;
	sdram->padcr = 0x01030203;

	sdram->cr00 = 0x01010101;
	sdram->cr01 = 0x00000101;
	sdram->cr02 = 0x01010100;
	sdram->cr03 = 0x01010000;
	sdram->cr04 = 0x00010101;
	sdram->cr06 = 0x00010100;
	sdram->cr07 = 0x00000001;
	sdram->cr08 = 0x01000001;
	sdram->cr09 = 0x00000100;
	sdram->cr10 = 0x00010001;
	sdram->cr11 = 0x00000200;
	sdram->cr12 = 0x01000002;
	sdram->cr13 = 0x00000000;
	sdram->cr14 = 0x00000100;
	sdram->cr15 = 0x02000100;
	sdram->cr16 = 0x02000407;
	sdram->cr17 = 0x02030007;
	sdram->cr18 = 0x02000100;
	sdram->cr19 = 0x0A030203;
	sdram->cr20 = 0x00020708;
	sdram->cr21 = 0x00050008;
/*	sdram->cr22 = 0x04030002;*/
	sdram->cr22 = 0x04020002;
	sdram->cr23 = 0x00000004;
	sdram->cr24 = 0x020A0000;
	sdram->cr25 = 0x0C00000E;
	sdram->cr26 = 0x00002004;
	sdram->cr28 = 0x00100010;
	sdram->cr29 = 0x00100010;
	sdram->cr31 = 0x07990000;
	sdram->cr40 = 0x00000000;
	sdram->cr41 = 0x00C80064;
	sdram->cr42 = 0x44520002;
	sdram->cr43 = 0x00C80023;
	sdram->cr45 = 0x0000C350;
	sdram->cr56 = 0x04000000;
/*	sdram->cr57 = 0x03000304; */
	sdram->cr57 = 0x03000305;
	sdram->cr58 = 0x40040000;
	sdram->cr59 = 0xC0004004;
	sdram->cr60 = 0x0642C000;
	sdram->cr61 = 0x00000642;
	asm("tpf");

	sdram->cr09 = 0x01000100;

/*	while ((sdram->cr27 & 0xC) != 8); */

	udelay(100);
#endif
	return (dramsize);
};
#if 0
u32 memTestDataBus(u32 adr)
{
	volatile u8 *address = (u8 *)adr;
	u8 pattern;
	u8 readback;

	printf("\nRunning memTestDataBus test ...\n");

	/* Perform a walking 1's test at the given address. */
	for (pattern = 1; pattern != 0; pattern <<= 1)
	{
		/* Write the test pattern. */
		*address = pattern;

		/* Read it back (immediately is okay for this test). */
		readback = *address;
		if (readback != pattern)
		{
			printf("memTestDataBus pattern %02x @ 0x%08x - 0x%x\n",
			       pattern, (u32)address, readback);
		}
	}

	printf("End memTestDataBus test\n");

	return (0);
}

u32 memTestAddressBus(u32 adr, u32 nBytes)
{
	volatile u8 *baseAddress = (u8 *)adr;
	u32 addressMask = (nBytes - 1);
	u32 offset;
	u32 testOffset;

	u8 pattern      = (u8) 0xAAAAAAAA;
	u8 antipattern  = (u8) 0x55555555;
	u8 readback8;

	printf("\nRunning memTestAddressBus test ...\n");

	/* Write the default pattern at each of the power-of-two offsets. */
	for (offset = sizeof(u8); (offset & addressMask) != 0; offset <<= 1)
		baseAddress[offset] = pattern;

	/* Check for address bits stuck high. */
	testOffset = 0;
	baseAddress[testOffset] = antipattern;
	printf("Check for stuck high\n");

	for (offset = sizeof(u8); (offset & addressMask) != 0; offset <<= 1)
	{
		readback8 = baseAddress[offset];
		if (readback8 != pattern)
		{
			printf("memTestAddressBus high @ addr:0x%08lx data:0x%02x pattern %0x02x\n",
			     (u32)(baseAddress+offset), readback8, pattern);
		}
	}

	baseAddress[testOffset] = pattern;
	printf("Check for stuck low\n");

	/* Check for address bits stuck low or shorted. */
	for (testOffset = sizeof(u8); (testOffset & addressMask) != 0; testOffset <<= 1)
	{
		baseAddress[testOffset] = antipattern;

		for (offset = sizeof(u8); (offset & addressMask) != 0; offset <<= 1)
		{
			readback8 = baseAddress[offset];

			if ((readback8 != pattern) && (offset != testOffset))
			{
				printf("memTestAddressBus low @ addr:0x%08lx data:0x%02x pattern 0x%02x\n",
				       (u32)(baseAddress+offset), readback8, pattern);
			}
		}

		baseAddress[testOffset] = pattern;
	}

	printf("End memTestAddressBus test\n");

	return (0);
}
#endif

int testdram(void)
{
#if 0
	memTestDataBus(CONFIG_SYS_MEMTEST_START);
	memTestAddressBus(CONFIG_SYS_MEMTEST_START, 0x2000);
#endif
	return (0);
}
