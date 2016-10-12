/*
 * Atmel SPI DataFlash support
 *
 * Copyright (C) 2008 Atmel Corporation
 *
 * Copyright (C) 2010 Freescale Semiconductor, Inc. All Rights Reserved.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

/* AT45-specific commands */
#define CMD_AT45_READ_STATUS		0xd7
#define CMD_AT45_ERASE_PAGE		0x81
#define CMD_AT45_LOAD_PROG_BUF1		0x82
#define CMD_AT45_LOAD_BUF1		0x84
#define CMD_AT45_LOAD_PROG_BUF2		0x85
#define CMD_AT45_LOAD_BUF2		0x87
#define CMD_AT45_PROG_BUF1		0x88
#define CMD_AT45_PROG_BUF2		0x89

#define CMD_AT26_READ_STATUS		0x05
#define CMD_AT26_WRITE_STATUS		0x01
#define CMD_AT26_WRITE_DIS		0x04
#define CMD_AT26_WRITE_EN		0x06
#define CMD_AT26_ERASE_BLOCK_4K		0x20
#define CMD_AT26_ERASE_BLOCK_32K	0x52
#define CMD_AT26_ERASE_BLOCK_64K	0xD8
#define CMD_AT26_PROG_PAGE		0x02
#define CMD_AT26_READ_ARRAY		0x0B
#define CMD_AT26_UNPROT_SECTOR		0x39
#define AT26_STATUS_READY		(0x1)

/* AT45 status register bits */
#define AT45_STATUS_P2_PAGE_SIZE	(1 << 0)
#define AT45_STATUS_READY		(1 << 7)

/* DataFlash family IDs, as obtained from the second idcode byte */
#define DF_FAMILY_AT26F			0
#define DF_FAMILY_AT45			1
#define DF_FAMILY_AT26DF		2	/* AT25DF and AT26DF */

struct atmel_spi_flash_params {
	u8		idcode1;
	/* Log2 of page size in power-of-two mode */
	u8		l2_page_size;
	u8		pages_per_block;
	u8		blocks_per_sector;
	u8		nr_sectors;
	const char	*name;
};

/* spi_flash needs to be first so upper layers can free() it */
struct atmel_spi_flash {
	struct spi_flash flash;
	const struct atmel_spi_flash_params *params;
};

static inline struct atmel_spi_flash *
to_atmel_spi_flash(struct spi_flash *flash)
{
	return container_of(flash, struct atmel_spi_flash, flash);
}

static const struct atmel_spi_flash_params atmel_spi_flash_table[] = {
	{
		.idcode1		= 0x45,
		.l2_page_size		= 8,
		.pages_per_block	= 128,
		.blocks_per_sector	= 2,
		.nr_sectors		= 16,
		.name			= "AT26DF081A",
	},
	{
		.idcode1		= 0x22,
		.l2_page_size		= 8,
		.pages_per_block	= 8,
		.blocks_per_sector	= 16,
		.nr_sectors		= 4,
		.name			= "AT45DB011D",
	},
	{
		.idcode1		= 0x23,
		.l2_page_size		= 8,
		.pages_per_block	= 8,
		.blocks_per_sector	= 16,
		.nr_sectors		= 8,
		.name			= "AT45DB021D",
	},
	{
		.idcode1		= 0x24,
		.l2_page_size		= 8,
		.pages_per_block	= 8,
		.blocks_per_sector	= 32,
		.nr_sectors		= 8,
		.name			= "AT45DB041D",
	},
	{
		.idcode1		= 0x25,
		.l2_page_size		= 8,
		.pages_per_block	= 8,
		.blocks_per_sector	= 32,
		.nr_sectors		= 16,
		.name			= "AT45DB081D",
	},
	{
		.idcode1		= 0x26,
		.l2_page_size		= 9,
		.pages_per_block	= 8,
		.blocks_per_sector	= 32,
		.nr_sectors		= 16,
		.name			= "AT45DB161D",
	},
	{
		.idcode1		= 0x27,
		.l2_page_size		= 9,
		.pages_per_block	= 8,
		.blocks_per_sector	= 64,
		.nr_sectors		= 64,
		.name			= "AT45DB321D",
	},
	{
		.idcode1		= 0x28,
		.l2_page_size		= 10,
		.pages_per_block	= 8,
		.blocks_per_sector	= 32,
		.nr_sectors		= 32,
		.name			= "AT45DB642D",
	},
};

static int at26_wait_ready(struct spi_flash *flash, unsigned long timeout)
{
	struct spi_slave *spi = flash->spi;
	unsigned long timebase;
	int ret;
	u8 cmd = CMD_AT26_READ_STATUS;
	u8 status;

	ret = spi_xfer(spi, 8, &cmd, NULL, SPI_XFER_BEGIN);
	if (ret) {
		debug("SF: Failed to send command %02x: %d\n", cmd, ret);
		return ret;
	}

	timebase = get_timer(0);
	do {
		ret = spi_xfer(spi, 8, NULL, &status, 0);
		if (ret)
			return -1;

		if ((status & AT26_STATUS_READY) == 0)
			break;

	} while (get_timer(timebase) < timeout);

	spi_xfer(spi, 0, NULL, NULL, SPI_XFER_END);

	if ((status & AT26_STATUS_READY) == 0)
		return 0;

	/* Timed out */
	return -1;
}

static int at45_wait_ready(struct spi_flash *flash, unsigned long timeout)
{
	struct spi_slave *spi = flash->spi;
	unsigned long timebase;
	int ret;
	u8 cmd = CMD_AT45_READ_STATUS;
	u8 status;

	timebase = get_timer(0);

	ret = spi_xfer(spi, 8, &cmd, NULL, SPI_XFER_BEGIN);
	if (ret)
		return -1;

	do {
		ret = spi_xfer(spi, 8, NULL, &status, 0);
		if (ret)
			return -1;

		if (status & AT45_STATUS_READY)
			break;
	} while (get_timer(timebase) < timeout);

	/* Deactivate CS */
	spi_xfer(spi, 0, NULL, NULL, SPI_XFER_END);

	if (status & AT45_STATUS_READY)
		return 0;

	/* Timed out */
	return -1;
}

/*
 * Assemble the address part of a command for AT45 devices in
 * non-power-of-two page size mode.
 */
static void at45_build_address(struct atmel_spi_flash *asf, u8 *cmd, u32 offset)
{
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long page_size;
	unsigned int page_shift;

	/*
	 * The "extra" space per page is the power-of-two page size
	 * divided by 32.
	 */
	page_shift = asf->params->l2_page_size;
	page_size = (1 << page_shift) + (1 << (page_shift - 5));
	page_shift++;
	page_addr = offset / page_size;
	byte_addr = offset % page_size;

	cmd[0] = page_addr >> (16 - page_shift);
	cmd[1] = page_addr << (page_shift - 8) | (byte_addr >> 8);
	cmd[2] = byte_addr;
}

static int dataflash_read_fast_p2(struct spi_flash *flash,
		u32 offset, size_t len, void *buf)
{
	u8 cmd[5];

	cmd[0] = CMD_READ_ARRAY_FAST;
	cmd[1] = offset >> 16;
	cmd[2] = offset >> 8;
	cmd[3] = offset;
	cmd[4] = 0x00;

	return spi_flash_read_common(flash, cmd, sizeof(cmd), buf, len);
}

static int dataflash_read_fast_at26(struct spi_flash *flash,
			     u32 offset, size_t len, void *buf)
{
	struct atmel_spi_flash *asf = to_atmel_spi_flash(flash);
	unsigned long page_addr;
	unsigned long page_size;
	u8 cmd[5];

	page_size = (1 << asf->params->l2_page_size);
	page_addr = offset / page_size;

	cmd[0] = CMD_READ_ARRAY_FAST;
	cmd[1] = page_addr >> 8;
	cmd[2] = page_addr;
	cmd[3] = offset % page_size;
	cmd[4] = 0x00;

	return spi_flash_read_common(flash, cmd, sizeof(cmd), buf, len);
}

static int dataflash_read_fast_at45(struct spi_flash *flash,
		u32 offset, size_t len, void *buf)
{
	struct atmel_spi_flash *asf = to_atmel_spi_flash(flash);
	u8 cmd[5];

	cmd[0] = CMD_READ_ARRAY_FAST;
	at45_build_address(asf, cmd + 1, offset);
	cmd[4] = 0x00;

	return spi_flash_read_common(flash, cmd, sizeof(cmd), buf, len);
}

/*
 * TODO: the two write funcs (_p2/_at45) should get unified ...
 */
static int dataflash_write_p2(struct spi_flash *flash,
		u32 offset, size_t len, const void *buf)
{
	struct atmel_spi_flash *asf = to_atmel_spi_flash(flash);
	unsigned long page_size;
	u32 addr = offset;
	size_t chunk_len;
	size_t actual;
	int ret;
	u8 cmd[4];

	/*
	 * TODO: This function currently uses only page buffer #1.  We can
	 * speed this up by using both buffers and loading one buffer while
	 * the other is being programmed into main memory.
	 */

	page_size = (1 << asf->params->l2_page_size);

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	for (actual = 0; actual < len; actual += chunk_len) {
		chunk_len = min(len - actual, page_size - (addr % page_size));

		/* Use the same address bits for both commands */
		cmd[0] = CMD_AT45_LOAD_BUF1;
		cmd[1] = addr >> 16;
		cmd[2] = addr >> 8;
		cmd[3] = addr;

		ret = spi_flash_cmd_write(flash->spi, cmd, 4,
				buf + actual, chunk_len);
		if (ret < 0) {
			debug("SF: Loading AT45 buffer failed\n");
			goto out;
		}

		cmd[0] = CMD_AT45_PROG_BUF1;
		ret = spi_flash_cmd_write(flash->spi, cmd, 4, NULL, 0);
		if (ret < 0) {
			debug("SF: AT45 page programming failed\n");
			goto out;
		}

		ret = at45_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret < 0) {
			debug("SF: AT45 page programming timed out\n");
			goto out;
		}

		addr += chunk_len;
	}

	debug("SF: AT45: Successfully programmed %zu bytes @ 0x%x\n",
			len, offset);
	ret = 0;

out:
	spi_release_bus(flash->spi);
	return ret;
}

static int dataflash_write_at26(struct spi_flash *flash,
		u32 offset, size_t len, const void *buf)
{
	struct atmel_spi_flash *asf = to_atmel_spi_flash(flash);
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long page_size;
	size_t chunk_len;
	size_t actual;
	int ret;
	u8 cmd[4];

	/*
	 * TODO: This function currently uses only page buffer #1.  We can
	 * speed this up by using both buffers and loading one buffer while
	 * the other is being programmed into main memory.
	 */

	page_size = (1 << asf->params->l2_page_size);
	page_addr = offset / page_size;
	byte_addr = offset % page_size;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}
#if 0
	ret = spi_flash_cmd(flash->spi, CMD_AT26_WRITE_EN, NULL, 0);
	if (ret < 0) {
		debug("SF: Enabling Write failed\n");
		return ret;
	}

	cmd[0] = CMD_AT26_WRITE_STATUS;
	cmd[1] = 0;
	ret = spi_flash_cmd_write(flash->spi, cmd, 2, NULL, 0);
	if (ret < 0) {
		debug("SF: AT26 page erase failed\n");
		return ret;
	}

	ret = at26_wait_ready(flash, SPI_FLASH_SECTOR_ERASE_TIMEOUT);
	if (ret < 0) {
		debug("SF: AT45 page programming timed out\n");
		return ret;
	}
#endif
	ret = 0;
	for (actual = 0; actual < len; actual += chunk_len) {
		chunk_len = min(len - actual, page_size - byte_addr);

		/* Use the same address bits for both commands */
		cmd[0] = CMD_AT26_PROG_PAGE;
		cmd[1] = page_addr >> 8;
		cmd[2] = page_addr;
		cmd[3] = byte_addr;

		ret = spi_flash_cmd(flash->spi, CMD_AT26_WRITE_EN, NULL, 0);
		if (ret < 0) {
			debug("SF: Enabling Write failed\n");
			break;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, 4,
				buf + actual, chunk_len);
		if (ret < 0) {
			debug("SF: Loading AT26 buffer failed\n");
			break;
		}

		ret = at26_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret < 0) {
			debug("SF: AT45 page programming timed out\n");
			break;
		}

		page_addr++;
		byte_addr = 0;
	}

	debug("SF: AT26: Successfully programmed %zu bytes @ 0x%x\n",
			len, offset);

	spi_release_bus(flash->spi);
	return ret;
}

static int dataflash_write_at45(struct spi_flash *flash,
		u32 offset, size_t len, const void *buf)
{
	struct atmel_spi_flash *asf = to_atmel_spi_flash(flash);
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long page_size;
	unsigned int page_shift;
	size_t chunk_len;
	size_t actual;
	int ret;
	u8 cmd[4];

	/*
	 * TODO: This function currently uses only page buffer #1.  We can
	 * speed this up by using both buffers and loading one buffer while
	 * the other is being programmed into main memory.
	 */

	page_shift = asf->params->l2_page_size;
	page_size = (1 << page_shift) + (1 << (page_shift - 5));
	page_shift++;
	page_addr = offset / page_size;
	byte_addr = offset % page_size;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	for (actual = 0; actual < len; actual += chunk_len) {
		chunk_len = min(len - actual, page_size - byte_addr);

		/* Use the same address bits for both commands */
		cmd[0] = CMD_AT45_LOAD_BUF1;
		cmd[1] = page_addr >> (16 - page_shift);
		cmd[2] = page_addr << (page_shift - 8) | (byte_addr >> 8);
		cmd[3] = byte_addr;

		ret = spi_flash_cmd_write(flash->spi, cmd, 4,
				buf + actual, chunk_len);
		if (ret < 0) {
			debug("SF: Loading AT45 buffer failed\n");
			goto out;
		}

		cmd[0] = CMD_AT45_PROG_BUF1;
		ret = spi_flash_cmd_write(flash->spi, cmd, 4, NULL, 0);
		if (ret < 0) {
			debug("SF: AT45 page programming failed\n");
			goto out;
		}

		ret = at45_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret < 0) {
			debug("SF: AT45 page programming timed out\n");
			goto out;
		}

		page_addr++;
		byte_addr = 0;
	}

	debug("SF: AT45: Successfully programmed %zu bytes @ 0x%x\n",
			len, offset);
	ret = 0;

out:
	spi_release_bus(flash->spi);
	return ret;
}

/*
 * TODO: the two erase funcs (_p2/_at45) should get unified ...
 */
int dataflash_erase_p2(struct spi_flash *flash, u32 offset, size_t len)
{
	struct atmel_spi_flash *asf = to_atmel_spi_flash(flash);
	unsigned long page_size;

	size_t actual;
	int ret;
	u8 cmd[4];

	/*
	 * TODO: This function currently uses page erase only. We can
	 * probably speed things up by using block and/or sector erase
	 * when possible.
	 */

	page_size = (1 << asf->params->l2_page_size);

	if (offset % page_size || len % page_size) {
		debug("SF: Erase offset/length not multiple of page size\n");
		return -1;
	}

	cmd[0] = CMD_AT45_ERASE_PAGE;
	cmd[3] = 0x00;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	for (actual = 0; actual < len; actual += page_size) {
		cmd[1] = offset >> 16;
		cmd[2] = offset >> 8;

		ret = spi_flash_cmd_write(flash->spi, cmd, 4, NULL, 0);
		if (ret < 0) {
			debug("SF: AT45 page erase failed\n");
			goto out;
		}

		ret = at45_wait_ready(flash, SPI_FLASH_PAGE_ERASE_TIMEOUT);
		if (ret < 0) {
			debug("SF: AT45 page erase timed out\n");
			goto out;
		}

		offset += page_size;
	}

	debug("SF: AT45: Successfully erased %zu bytes @ 0x%x\n",
			len, offset);
	ret = 0;

out:
	spi_release_bus(flash->spi);
	return ret;
}

int dataflash_erase_at26(struct spi_flash *flash, u32 offset, size_t len)
{
	struct atmel_spi_flash *asf = to_atmel_spi_flash(flash);
	unsigned long sector_size;
	unsigned long page_size;

	size_t actual;
	int ret;
	u8 cmd[4];

	/*
	 * TODO: This function currently uses page erase only. We can
	 * probably speed things up by using block and/or sector erase
	 * when possible.
	 */

	page_size = (1 << asf->params->l2_page_size);
	sector_size = page_size * asf->params->pages_per_block * asf->params->blocks_per_sector;

	if (offset % page_size || len % page_size) {
		debug("SF: Erase offset/length not multiple of page size\n");
		return -1;
	}

	len /= sector_size;
	cmd[2] = 0x00;
	cmd[3] = 0x00;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	ret = spi_flash_cmd(flash->spi, CMD_AT26_WRITE_EN, NULL, 0);
	if (ret < 0) {
		debug("SF: Enabling Write failed\n");
		return ret;
	}

	/* Global unprotect all sectors */
	cmd[0] = CMD_AT26_WRITE_STATUS;
	cmd[1] = 0;
	ret = spi_flash_cmd_write(flash->spi, cmd, 2, NULL, 0);
	if (ret < 0) {
		debug("SF: AT26 page erase failed\n");
		return ret;
	}

	ret = at26_wait_ready(flash, SPI_FLASH_SECTOR_ERASE_TIMEOUT);
	if (ret < 0) {
		debug("SF: AT45 page programming timed out\n");
		return ret;
	}

	ret = 0;
	cmd[0] = CMD_AT26_ERASE_BLOCK_64K;
	for (actual = 0; actual < len; actual++) {
		cmd[1] = (offset / sector_size) + actual;
		ret = spi_flash_cmd(flash->spi, CMD_AT26_WRITE_EN, NULL, 0);
		if (ret < 0) {
			debug("SF: Enabling Write failed\n");
			break;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, 4, NULL, 0);
		if (ret < 0) {
			debug("SF: AT26 page erase failed\n");
			break;
		}

		ret = at26_wait_ready(flash, SPI_FLASH_PAGE_ERASE_TIMEOUT);
		if (ret < 0) {
			debug("SF: AT26 page erase timed out\n");
			break;
		}
	}

	debug("SF: AT26: Successfully erased %zu bytes @ 0x%x\n",
	    len * sector_size, offset);

	spi_release_bus(flash->spi);
	return ret;
}

int dataflash_erase_at45(struct spi_flash *flash, u32 offset, size_t len)
{
	struct atmel_spi_flash *asf = to_atmel_spi_flash(flash);
	unsigned long page_addr;
	unsigned long page_size;
	unsigned int page_shift;
	size_t actual;
	int ret;
	u8 cmd[4];

	/*
	 * TODO: This function currently uses page erase only. We can
	 * probably speed things up by using block and/or sector erase
	 * when possible.
	 */

	page_shift = asf->params->l2_page_size;
	page_size = (1 << page_shift) + (1 << (page_shift - 5));
	page_shift++;
	page_addr = offset / page_size;

	if (offset % page_size || len % page_size) {
		debug("SF: Erase offset/length not multiple of page size\n");
		return -1;
	}

	cmd[0] = CMD_AT45_ERASE_PAGE;
	cmd[3] = 0x00;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	for (actual = 0; actual < len; actual += page_size) {
		cmd[1] = page_addr >> (16 - page_shift);
		cmd[2] = page_addr << (page_shift - 8);

		ret = spi_flash_cmd_write(flash->spi, cmd, 4, NULL, 0);
		if (ret < 0) {
			debug("SF: AT45 page erase failed\n");
			goto out;
		}

		ret = at45_wait_ready(flash, SPI_FLASH_PAGE_ERASE_TIMEOUT);
		if (ret < 0) {
			debug("SF: AT45 page erase timed out\n");
			goto out;
		}

		page_addr++;
	}

	debug("SF: AT45: Successfully erased %zu bytes @ 0x%x\n",
			len, offset);
	ret = 0;

out:
	spi_release_bus(flash->spi);
	return ret;
}

struct spi_flash *spi_flash_probe_atmel(struct spi_slave *spi, u8 *idcode)
{
	const struct atmel_spi_flash_params *params;
	unsigned long page_size;
	unsigned int family;
	struct atmel_spi_flash *asf;
	unsigned int i;
	int ret;
	u8 status;

	for (i = 0; i < ARRAY_SIZE(atmel_spi_flash_table); i++) {
		params = &atmel_spi_flash_table[i];
		if (params->idcode1 == idcode[1])
			break;
	}

	if (i == ARRAY_SIZE(atmel_spi_flash_table)) {
		debug("SF: Unsupported DataFlash ID %02x\n",
				idcode[1]);
		return NULL;
	}

	asf = malloc(sizeof(struct atmel_spi_flash));
	if (!asf) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	asf->params = params;
	asf->flash.spi = spi;
	asf->flash.name = params->name;

	/* Assuming power-of-two page size initially. */
	page_size = 1 << params->l2_page_size;

	family = idcode[1] >> 5;

	switch (family) {
	case DF_FAMILY_AT45:
		/*
		 * AT45 chips have configurable page size. The status
		 * register indicates which configuration is active.
		 */
		ret = spi_flash_cmd(spi, CMD_AT45_READ_STATUS, &status, 1);
		if (ret)
			goto err;

		debug("SF: AT45 status register: %02x\n", status);

		if (!(status & AT45_STATUS_P2_PAGE_SIZE)) {
			asf->flash.read = dataflash_read_fast_at45;
			asf->flash.write = dataflash_write_at45;
			asf->flash.erase = dataflash_erase_at45;
			page_size += 1 << (params->l2_page_size - 5);
		} else {
			asf->flash.read = dataflash_read_fast_p2;
			asf->flash.write = dataflash_write_p2;
			asf->flash.erase = dataflash_erase_p2;
		}

		break;

	case DF_FAMILY_AT26F:
	case DF_FAMILY_AT26DF:
		asf->flash.read = dataflash_read_fast_at26;
		asf->flash.write = dataflash_write_at26;
		asf->flash.erase = dataflash_erase_at26;
		break;

	default:
		debug("SF: Unsupported DataFlash family %u\n", family);
		goto err;
	}

	asf->flash.size = page_size * params->pages_per_block
				* params->blocks_per_sector
				* params->nr_sectors;

	debug("SF: Detected %s with page size %lu, total %u bytes\n",
			params->name, page_size, asf->flash.size);

	return &asf->flash;

err:
	free(asf);
	return NULL;
}
