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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

/* W25XXX-specific commands */
#define CMD_W25XXX_WREN		0x06	/* Write Enable */
#define CMD_W25XXX_WRDI		0x04	/* Write Disable */
#define CMD_W25XXX_RDSR		0x05	/* Read Status Register */
#define CMD_W25XXX_WRSR		0x01	/* Write Status Register */
#define CMD_W25XXX_READ		0x03	/* Read Data Bytes */
#define CMD_W25XXX_FAST_READ	0x0b	/* Read Data Bytes at Higher Speed */
#define CMD_W25XXX_FAST_READ_D	0x3b	/* Read Data Bytes at Higher Speed Dual Output */
#define CMD_W25XXX_PP		0x02	/* Page Program */
#define CMD_W25XXX_SE		0x20	/* Sector Erase */
#define CMD_W25XXX_BLK		0xd8	/* Block Erase */
#define CMD_W25XXX_CHIP		0xc7	/* Chip Erase */
#define CMD_W25XXX_DP		0xb9	/* Deep Power-down */
#define CMD_W25XXX_RES		0xab	/* Release from DP, and Read Signature */
#define CMD_W25XXX_DEV_ID	0x90	/* Manufacturer / Device ID */
#define CMD_W25XXX_JEDEC_ID	0x9f	/* Jedec ID */

#define WBD_ID_W25X10		0x10
#define WBD_ID_W25X20		0x11
#define WBD_ID_W25X40		0x12
#define WBD_ID_W25X80		0x13

#define winbond_SR_WIP		(1 << 0)	/* Write-in-Progress */

struct winbond_spi_flash_params {
	u8 idcode1;
	u16 page_size;
	u16 pages_per_sector;
	u16 nr_sectors;
	const char *name;
};

/* spi_flash needs to be first so upper layers can free() it */
struct winbond_spi_flash {
	struct spi_flash flash;
	const struct winbond_spi_flash_params *params;
};

static inline struct winbond_spi_flash *to_winbond_spi_flash(struct spi_flash
							     *flash)
{
	return container_of(flash, struct winbond_spi_flash, flash);
}

static const struct winbond_spi_flash_params winbond_spi_flash_table[] = {
	{
		.idcode1 = WBD_ID_W25X10,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 32,
		.name = "W25X16",
	},
	{
		.idcode1 = WBD_ID_W25X20,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 4,
		.name = "W25X20",
	},
	{
		.idcode1 = WBD_ID_W25X40,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 8,
		.name = "W25X40",
	},
	{
		.idcode1 = WBD_ID_W25X80,
		.page_size = 256,
		.pages_per_sector = 256,
		.nr_sectors = 16,
		.name = "W25X80",
	},
};

static int winbond_wait_ready(struct spi_flash *flash, unsigned long timeout)
{
	struct spi_slave *spi = flash->spi;
	unsigned long timebase;
	int ret;
	u8 cmd = CMD_W25XXX_RDSR;
	u8 status;

	ret = spi_xfer(spi, 8, &cmd, NULL, SPI_XFER_BEGIN);
	if (ret) {
		printf("SF: Failed to send command %02x: %d\n", cmd, ret);
		return ret;
	}

	timebase = get_timer(0);
	do {
		ret = spi_xfer(spi, 8, NULL, &status, 0);
		if (ret)
			return -1;

		if ((status & winbond_SR_WIP) == 0)
			break;

	} while (get_timer(timebase) < timeout);

	spi_xfer(spi, 0, NULL, NULL, SPI_XFER_END);

	if ((status & winbond_SR_WIP) == 0)
		return 0;

	/* Timed out */
	return -1;
}

static int winbond_read_fast(struct spi_flash *flash,
			     u32 offset, size_t len, void *buf)
{
	struct winbond_spi_flash *wbd = to_winbond_spi_flash(flash);
	unsigned long page_addr;
	unsigned long page_size;
	u8 cmd[5];

	page_size = wbd->params->page_size;
	page_addr = offset / page_size;

	cmd[0] = CMD_READ_ARRAY_FAST;
	cmd[1] = page_addr >> 8;
	cmd[2] = page_addr;
	cmd[3] = offset % page_size;
	cmd[4] = 0x00;

	return spi_flash_read_common(flash, cmd, sizeof(cmd), buf, len);
}

static int winbond_write(struct spi_flash *flash,
			 u32 offset, size_t len, const void *buf)
{
	struct winbond_spi_flash *wbd = to_winbond_spi_flash(flash);
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long page_size;
	size_t chunk_len;
	size_t actual;
	int ret;
	u8 cmd[4];

	page_size = wbd->params->page_size;
	page_addr = offset / page_size;
	byte_addr = offset % page_size;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		printf("SF: Unable to claim SPI bus\n");
		return ret;
	}

	ret = 0;
	for (actual = 0; actual < len; actual += chunk_len) {
		chunk_len = min(len - actual, page_size - byte_addr);

		cmd[0] = CMD_W25XXX_PP;
		cmd[1] = page_addr >> 8;
		cmd[2] = page_addr;
		cmd[3] = byte_addr;

		printf
		    ("PP: 0x%p => cmd = { 0x%02x 0x%02x%02x%02x } chunk_len = %d\n",
		     buf + actual, cmd[0], cmd[1], cmd[2], cmd[3], chunk_len);

		ret = spi_flash_cmd(flash->spi, CMD_W25XXX_WREN, NULL, 0);
		if (ret < 0) {
			printf("SF: Enabling Write failed\n");
			break;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, 4,
					  buf + actual, chunk_len);
		if (ret < 0) {
			printf("SF: winbond Page Program failed\n");
			break;
		}

		ret = winbond_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret < 0) {
			printf("SF: winbond page programming timed out\n");
			break;
		}

		page_addr++;
		byte_addr = 0;
	}

	printf("SF: winbond: Successfully programmed %u bytes @ 0x%x\n",
	      len, offset);

	spi_release_bus(flash->spi);
	return ret;
}

int winbond_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	struct winbond_spi_flash *wbd = to_winbond_spi_flash(flash);
	unsigned long sector_size;
	size_t actual;
	int ret;
	u8 cmd[4];

	/*
	 * This function currently uses sector erase only.
	 * probably speed things up by using bulk erase
	 * when possible.
	 */

	sector_size = wbd->params->page_size * wbd->params->pages_per_sector;

	if (offset % sector_size || len % sector_size) {
		printf("SF: Erase offset/length not multiple of sector size\n");
		return -1;
	}

	len /= sector_size;
	cmd[0] = CMD_W25XXX_SE;
	cmd[2] = 0x00;
	cmd[3] = 0x00;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		printf("SF: Unable to claim SPI bus\n");
		return ret;
	}

	ret = 0;
	for (actual = 0; actual < len; actual++) {
		cmd[1] = (offset / sector_size) + actual;

		ret = spi_flash_cmd(flash->spi, CMD_W25XXX_WREN, NULL, 0);
		if (ret < 0) {
			printf("SF: Enabling Write failed\n");
			break;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, 4, NULL, 0);
		if (ret < 0) {
			printf("SF: winbond page erase failed\n");
			break;
		}

		ret = winbond_wait_ready(flash, SPI_FLASH_PAGE_ERASE_TIMEOUT);
		if (ret < 0) {
			printf("SF: winbond page erase timed out\n");
			break;
		}
	}

	printf("SF: winbond: Successfully erased %u bytes @ 0x%x\n",
	      len * sector_size, offset);

	spi_release_bus(flash->spi);
	return ret;
}

struct spi_flash *spi_flash_probe_winbond(struct spi_slave *spi, u8 * idcode)
{
	const struct winbond_spi_flash_params *params;
	struct winbond_spi_flash *wbd;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(winbond_spi_flash_table); i++) {
		params = &winbond_spi_flash_table[i];
		if (params->idcode1 == idcode[2]) {
			break;
		}
	}

	if (i == ARRAY_SIZE(winbond_spi_flash_table)) {
		printf("SF: Unsupported winbond ID %02x\n", idcode[1]);
		return NULL;
	}

	wbd = malloc(sizeof(struct winbond_spi_flash));
	if (!wbd) {
		printf("SF: Failed to allocate memory\n");
		return NULL;
	}

	wbd->params = params;
	wbd->flash.spi = spi;
	wbd->flash.name = params->name;

	wbd->flash.write = winbond_write;
	wbd->flash.erase = winbond_erase;
	wbd->flash.read = winbond_read_fast;
	wbd->flash.size = params->page_size * params->pages_per_sector
	    * params->nr_sectors;

	printf("SF: Detected %s with page size %u, total %u bytes\n",
	      params->name, params->page_size, wbd->flash.size);

	return &wbd->flash;
}
