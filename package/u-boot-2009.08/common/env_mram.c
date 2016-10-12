/*
 * Copyright (C) 2010 Freescale Semiconductor, Inc. All Rights Reserved
 * Jason Jin <Jason.jin@freescale.com>

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
#include <command.h>
#include <environment.h>
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

/* references to names in env_common.c */
extern uchar default_environment[];
char *env_name_spec = "MRAM";
env_t *env_ptr = (env_t *)CONFIG_ENV_ADDR;

uchar env_get_char_spec(int index)
{
	return ( *((uchar *)(gd->env_addr + index)) );
}

void env_relocate_spec(void)
{
	memcpy (env_ptr, (char*)CONFIG_ENV_ADDR , CONFIG_ENV_SIZE);
}


int saveenv(void)
{
	volatile fbcs_t *fbcs = (fbcs_t *) MMAP_FBCS;

	/*Protect off mram*/
	fbcs->csmr0 &= ~0x100;

	if (memcpy ((char*)CONFIG_ENV_ADDR, env_ptr, CONFIG_ENV_SIZE) == NULL)
		return 1;

	/*Protect on mram*/
	fbcs->csmr0 |= 0x100;

	return 0;
}


 /* Initialize Environment use */
int  env_init(void)
{
	ulong crc = *(ulong *) CONFIG_ENV_ADDR;

	if (crc32(0, env_ptr->data, ENV_SIZE) == crc) {
		gd->env_addr  = (ulong)CONFIG_ENV_ADDR + sizeof(ulong);
		gd->env_valid = 1;
	} else {
		gd->env_addr  = (ulong)&default_environment[0];
		gd->env_valid = 0;
	}

	return (0);
}

int do_protect_mram(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	volatile fbcs_t *fbcs = (fbcs_t *) MMAP_FBCS;

	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}

	if (strcmp(argv[1], "off") == 0) {
		fbcs->csmr0 &= ~0x100;
	} else if (strcmp(argv[1], "on") == 0) {
		fbcs->csmr0 |= 0x100;
	} else {
		cmd_usage(cmdtp);
		return 1;
	}
}

U_BOOT_CMD(
	protect,  3,  0,   do_protect_mram,
	"enable or disable MRAM write protection",
	"on mram\n    - protect all the mram space\n"
	"protect off mram\n    - make all the mram writable\n"
);
