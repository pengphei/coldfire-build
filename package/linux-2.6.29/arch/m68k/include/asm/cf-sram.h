/*
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Author: Lanttor.Guo@freescale.com
 *
 * Providing on-chip SRAM allocation and free APIs to kernel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CF_SRAM_H
#define CF_SRAM_H

extern int declare_sram_pool(void *start, size_t size);

extern void *sram_alloc(size_t len);
extern void sram_free(void *addr, size_t len);

#endif
