/*
 *  linux/arch/m68k/coldfire/config_m5445x.c
 *
 * Copyright(C) 2007-2009 Freescale Semiconductor, Inc. All rights reserved.
 * Author:
 * 	Kurt Mahan kmahan@freescale.com
 * 	Matt Waddel Matt.Waddel@freescale.com
 * 	Jason Jin Jason.Jin@freescale.com
 * 	Shrek Wu B16972@freescale.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/console.h>
#include <linux/bootmem.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/clockchips.h>
#include <asm/bootinfo.h>
#include <asm/machdep.h>
#include <asm/coldfire.h>
#include <asm/cfcache.h>
#include <asm/cacheflush.h>
#include <asm/io.h>
#include <asm/cfmmu.h>
#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/traps.h>
#include <asm/movs.h>
#include <asm/movs.h>
#include <asm/page.h>
#include <asm/pgalloc.h>
#include <asm/cf-sram.h>

#include <asm/mcfsim.h>

#define UBOOT_EXTRA_CLOCKS
#include <asm/bootinfo.h>

#include <asm/mcf5445x_intc.h>
#include <asm/mcf5445x_sdramc.h>
#include <asm/mcf5445x_fbcs.h>
#include <asm/mcf5445x_dtim.h>
#include <asm/mcf5445x_xbs.h>

extern int get_irq_list(struct seq_file *p, void *v);
extern char _text, _end;
extern char _etext, _edata, __init_begin, __init_end;
extern struct console mcfrs_console;
extern char m68k_command_line[CL_SIZE];
extern unsigned long availmem;
extern unsigned long shadow_cacr;
EXPORT_SYMBOL(shadow_cacr);

static int irq_enable[NR_IRQS];
unsigned long num_pages;

/* cf dma physical addresses */
unsigned long cf_dma_base;
unsigned long cf_dma_end;
unsigned long cf_dma_size;
EXPORT_SYMBOL(cf_dma_base);
EXPORT_SYMBOL(cf_dma_end);
EXPORT_SYMBOL(cf_dma_size);

/* ethernet mac addresses from uboot */
unsigned char uboot_enet0[6];
unsigned char uboot_enet1[6];

unsigned long u_initrd_start;
unsigned long u_initrd_end;

void coldfire_sort_memrec(void)
{
	int i, j;

	/* Sort the m68k_memory records by address */
	for (i = 0; i < m68k_num_memory; ++i) {
		for (j = i + 1; j < m68k_num_memory; ++j) {
			if (m68k_memory[i].addr > m68k_memory[j].addr) {
				struct mem_info tmp;
				tmp = m68k_memory[i];
				m68k_memory[i] = m68k_memory[j];
				m68k_memory[j] = tmp;
			}
		}
	}
	/* Trim off discontiguous bits */
	for (i = 1; i < m68k_num_memory; ++i) {
		if ((m68k_memory[i-1].addr + m68k_memory[i-1].size) !=
			m68k_memory[i].addr) {
			printk(KERN_DEBUG "m68k_parse_bootinfo: "
				"addr gap between 0x%lx & 0x%lx\n",
				m68k_memory[i-1].addr+m68k_memory[i-1].size,
				m68k_memory[i].addr);
			m68k_num_memory = i;
			break;
		}
	}
}

/*
 * UBoot Handler
 */
int __init uboot_commandline(char *bootargs)
{
	int len = 0, cmd_line_len;
	static struct uboot_record uboot_info;
	u32 offset = PAGE_OFFSET_RAW - PHYS_OFFSET;

	extern unsigned long uboot_info_stk;

	/* validate address */
	if ((uboot_info_stk < PAGE_OFFSET_RAW) ||
	    (uboot_info_stk >= (PAGE_OFFSET_RAW + CONFIG_SDRAM_SIZE)))
		return 0;

	/* Add offset to get post-remapped kernel memory location */
	uboot_info.bdi = (struct bd_info *)
		((*(u32 *)(uboot_info_stk)) + offset);
	uboot_info.initrd_start = (*(u32 *)(uboot_info_stk+4)) + offset;
	uboot_info.initrd_end = (*(u32 *)(uboot_info_stk+8)) + offset;
	uboot_info.cmd_line_start = (*(u32 *)(uboot_info_stk+12)) + offset;
	uboot_info.cmd_line_stop = (*(u32 *)(uboot_info_stk+16)) + offset;

	u_initrd_start = uboot_info.initrd_start;
	u_initrd_end = uboot_info.initrd_end;

	/* copy over mac addresses */
	memcpy(uboot_enet0, uboot_info.bdi->bi_enet0addr, 6);
	memcpy(uboot_enet1, uboot_info.bdi->bi_enet1addr, 6);

	/* copy command line */
	cmd_line_len = uboot_info.cmd_line_stop - uboot_info.cmd_line_start;
	if ((cmd_line_len > 0) && (cmd_line_len < CL_SIZE-1))
		len = (int)strncpy(bootargs, (char *)uboot_info.cmd_line_start,\
				   cmd_line_len);

	return len;
}

#if defined(CONFIG_M54451)
#define DEFAULT_COMMAND_LINE \
	"debug root=/dev/nfs rw \
	nfsroot=172.27.155.1:/tftpboot/redstripe/rootfs/ \
	ip=172.27.155.51:172.27.155.1"
#elif defined(CONFIG_M54455)
#define MTD_DEFAULT_COMMAND_LINE \
	"root=/dev/mtdblock1 rw rootfstype=jffs2 \
	ip=none mtdparts=physmap-flash.0:5M(kernel)ro,-(jffs2)"
#define DEFAULT_COMMAND_LINE \
	"debug root=/dev/nfs rw \
	nfsroot=172.27.155.1:/tftpboot/redstripe/rootfs/ \
	ip=172.27.155.55:172.27.155.1"
#endif


/*
 * Early Coldfire Initialization.
 */
asmlinkage void __init cf_early_init(void)
{
	struct bi_record *record = (struct bi_record *) &_end;

	extern char _end;

	SET_VBR((void *)MCF_RAMBAR1);

	/* Mask all interrupts */
	MCF_INTC0_IMRL = 0xFFFFFFFF;
	MCF_INTC0_IMRH = 0xFFFFFFFF;
	MCF_INTC1_IMRL = 0xFFFFFFFF;
	MCF_INTC1_IMRH = 0xFFFFFFFF;

#if defined(CONFIG_NOR_FLASH_BASE)
	MCF_FBCS_CSAR(1) = CONFIG_NOR_FLASH_BASE;
#else
	MCF_FBCS_CSAR(1) = 0x00000000;
#endif

#if CONFIG_SDRAM_SIZE > (256*1024*1024)
	/* Init optional SDRAM chip select */
	MCF_SDRAMC_SDCS(1) = (256*1024*1024) | 0x1B;
#endif

	/* Setup SDRAM crossbar(XBS) priorities */
	MCF_XBS_PRS2 = (MCF_XBS_PRS_M0(MCF_XBS_PRI_2) |  /*CPU*/
			MCF_XBS_PRS_M1(MCF_XBS_PRI_3) |  /*eDMA*/
			MCF_XBS_PRS_M2(MCF_XBS_PRI_1) |  /*FEC0*/
			MCF_XBS_PRS_M3(MCF_XBS_PRI_5) |  /*FEC1*/
			MCF_XBS_PRS_M5(MCF_XBS_PRI_6) |  /*PCI*/
			MCF_XBS_PRS_M6(MCF_XBS_PRI_4) |  /*USB*/
			MCF_XBS_PRS_M7(MCF_XBS_PRI_7));  /*SBF*/

	m68k_machtype = MACH_CFMMU;
	m68k_fputype = FPU_CFV4E;
	m68k_mmutype = MMU_CFV4E;
	m68k_cputype = CPU_CFV4E;

	/* initialize PHYSRAM */
	m68k_num_memory = 0;
	m68k_memory[m68k_num_memory].addr = CONFIG_SDRAM_BASE;	/* phys */
	m68k_memory[m68k_num_memory++].size = CONFIG_SDRAM_SIZE;

	if (!uboot_commandline(m68k_command_line)) {
#if defined(CONFIG_BOOTPARAM)
		strncpy(m68k_command_line, CONFIG_BOOTPARAM_STRING, CL_SIZE-1);
#else
		strcpy(m68k_command_line, DEFAULT_COMMAND_LINE);
#endif
	}

#if defined(CONFIG_BLK_DEV_INITRD)
	/* add initrd image */
	record = (struct bi_record *) ((void *)record + record->size);
	record->tag = BI_RAMDISK;
	record->data[0] = u_initrd_start;
	record->data[1] = (u_initrd_end - u_initrd_start) ? u_initrd_end - u_initrd_start + 1 : 0;
	record->size =  sizeof(record->tag) + sizeof(record->size)
		+ sizeof(record->data[0]) + sizeof(record->data[1]);
#endif

	/* Mark end of tags. */
	record = (struct bi_record *) ((void *) record + record->size);
	record->tag = 0;
	record->data[0] = 0;
	record->data[1] = 0;
	record->size = sizeof(record->tag) + sizeof(record->size)
		+ sizeof(record->data[0]) + sizeof(record->data[1]);

	/* Invalidate caches via CACR */
	flush_bcache();
	cacr_set(CACHE_DISABLE_MODE);

	/* Turn on caches via CACR, enable EUSP */
	cacr_set(CACHE_INITIAL_MODE);

}

/* Assembler routines */
asmlinkage void buserr(void);
asmlinkage void trap(void);
asmlinkage void system_call(void);
asmlinkage void inthandler(void);

void __init coldfire_trap_init(void)
{
	int i = 0;
	e_vector *vectors;

	vectors = (e_vector *)MCF_RAMBAR1;
	/*
	 * There is a common trap handler and common interrupt
	 * handler that handle almost every vector. We treat
	 * the system call and bus error special, they get their
	 * own first level handlers.
	 */
	for (i = 3; (i <= 23); i++)
		vectors[i] = trap;
	for (i = 33; (i <= 63); i++)
		vectors[i] = trap;
	for (i = 24; (i <= 31); i++)
		vectors[i] = inthandler;
	for (i = 64; (i < 255); i++)
		vectors[i] = inthandler;

	vectors[255] = 0;
	vectors[2] = buserr;
	vectors[32] = system_call;
}

void settimericr(unsigned int timer, unsigned int level)
{
	volatile unsigned char *icrp;
	unsigned int icr = 0;
	unsigned char irq = 0;

	if (timer <= 3) {
		switch (timer) {
		case 0:
			irq = 32;
			icr = MCFSIM_ICR_TIMER0;
			break;
		case 1:
			irq = 33;
			icr = MCFSIM_ICR_TIMER1;
			break;
		case 2:
			irq = 34;
			icr = MCFSIM_ICR_TIMER2;
			break;
		case 3:
			irq = 35;
			icr = MCFSIM_ICR_TIMER3;
			break;
		}

		icrp = (volatile unsigned char *) (icr);
		*icrp = level;
		coldfire_enable_irq0(irq);
	}
}

void coldfire_sched_clk_init(void);

void coldfire_tick(void)
{
	/* Reset the ColdFire timer */
	__raw_writeb(MCF_DTIM_DTER_CAP | MCF_DTIM_DTER_REF, MCF_DTIM0_DTER);
}

void __init coldfire_sched_init(irq_handler_t handler)
{
	unsigned int	mcf_timerlevel = 5;
	unsigned int	mcf_timervector = 64+32;

	__raw_writew(MCF_DTIM_DTMR_RST_RST, MCF_DTIM0_DTMR);
	__raw_writel(((MCF_BUSCLK / 16) / HZ), MCF_DTIM0_DTRR);
	__raw_writew(MCF_DTIM_DTMR_ORRI	| MCF_DTIM_DTMR_CLK_DIV16 |
		     MCF_DTIM_DTMR_FRR	| MCF_DTIM_DTMR_RST_EN, \
		     MCF_DTIM0_DTMR);

	request_irq(mcf_timervector, handler, IRQF_DISABLED, \
		    "timer", (void *)MCF_DTIM0_DTMR);

	settimericr(0, mcf_timerlevel);

	/* initialize the fast clock */
	coldfire_sched_clk_init();
}

int timerirqpending(int timer)
{
	unsigned int imr = 0;
	unsigned ret = 0;

	switch (timer) {
	case 1:
		imr = 0x1;
		break;
	case 2:
		imr = 0x2;
		break;
	default:
		break;
	}

	ret = getiprh() & imr;
	return ret;
}

unsigned long coldfire_gettimeoffset(void)
{
	volatile unsigned long trr, tcn, offset;

	tcn = __raw_readw(MCF_DTIM0_DTCN);
	trr = __raw_readl(MCF_DTIM0_DTRR);
	offset = (tcn * (1000000 / HZ)) / trr;

	/* Check if we just wrapped the counters and maybe missed a tick */
	if ((offset < (1000000 / HZ / 2)) && timerirqpending(1))
		offset += 1000000 / HZ;
	return offset;
}

/* sched_clock support */

static unsigned long long sched_clk_val;

static irqreturn_t coldfire_sched_clk_irq(int irq, void *dev)
{
	__raw_writeb(MCF_DTIM_DTER_CAP | MCF_DTIM_DTER_REF, MCF_DTIM3_DTER);
	sched_clk_val += 0x100000000LL;

	return IRQ_HANDLED;
}

/*
 * Setup DTIM3 as the 5445x sched_clock() implementation.  The
 * input frequency to the clock is the internal bus clock (MCF_BUSCLK)
 * which is the system clock (MCF_CLK) / 2.
 *
 * On a system running at 266Mhz:
 *	MCF_CLK = 266Mhz (266000000)
 *	MCF_BUSCLK = 133Mhz (133000000)
 *	DTIM3 Tick = 133Mhz (133000000)
 *	Tick Resolution = 7.5ns (round to 8ns/Tick)
 *
 * On a system running at 240Mhz:
 *	MCF_CLK = 240Mhz (240000000)
 *	MCF_BUSCLK = 120Mhz (120000000)
 *	DTIM3 Tick = 120Mhz (120000000)
 *	Tick Resolution = 8.4ns (round to 8ns/Tick)
 */
void __init coldfire_sched_clk_init(void)
{
	unsigned int mcf_timerlevel = 5;
	unsigned int mcf_timervector = 64+32+3; /* DTIM3 */

	printk(KERN_INFO "Initializing DTIM3 for sched_clock\n");
	__raw_writew(MCF_DTIM_DTMR_RST_RST, MCF_DTIM3_DTMR);
	__raw_writel(0xffffffff, MCF_DTIM3_DTRR);
	__raw_writew(MCF_DTIM_DTMR_ORRI | MCF_DTIM_DTMR_CLK_DIV1 | \
		     MCF_DTIM_DTMR_RST_EN, MCF_DTIM3_DTMR);

	sched_clk_val = 0;
	request_irq(mcf_timervector, coldfire_sched_clk_irq, IRQF_DISABLED, \
		    "timer", (void *)MCF_DTIM3_DTMR);

	settimericr(3, mcf_timerlevel);
}

#ifndef CONFIG_GENERIC_CLOCKEVENTS
/* get highres timer */
unsigned long long sched_clock(void)
{
	unsigned long long ret = 0;
	ret = sched_clk_val | (unsigned long long)(MCF_REG32(MCF_DTIM3_DTCN));
	ret = ret << 3;
	return ret;
}

#else /*CONFIG_GENERIC_CLOCKEVENTS*/

static unsigned long long sched_dtim_clk_val =0;

unsigned long long sched_clock(void)
{
	unsigned long flags;
	unsigned long long tcn, cycles;

	local_irq_save(flags);
	tcn = ((unsigned long long)(MCF_REG32(MCF_DTIM2_DTCN)));
	cycles = sched_dtim_clk_val;
	local_irq_restore(flags);
	return cycles + tcn;
}

unsigned long long sys_dtim2_read(void)
{
	unsigned long flags;
	unsigned long long tcn, cycles;

	local_irq_save(flags);
	tcn = ((unsigned long long)(MCF_REG32(MCF_DTIM2_DTCN)));
	cycles = sched_dtim_clk_val;
	local_irq_restore(flags);

	return cycles + tcn;
}

static irqreturn_t coldfire_dtim_clk_irq(int irq, void *dev)
{
	struct clock_event_device *evt = (struct clock_event_device *)dev;

	__raw_writeb(MCF_DTIM_DTER_CAP | MCF_DTIM_DTER_REF, MCF_DTIM2_DTER);
	sched_dtim_clk_val +=  (MCF_BUSCLK / 16) / HZ;;
	evt->event_handler(evt);
	return IRQ_HANDLED;
}

void sys_dtim2_init(struct clock_event_device *evt)
{
	unsigned int mcf_timerlevel = 5;
	unsigned int mcf_timervector = 64+32+2; /* DTIM2 */

	printk(KERN_INFO "Initializing DTIM2 for sched_clock\n");
	__raw_writew(MCF_DTIM_DTMR_RST_RST, MCF_DTIM2_DTMR);
	__raw_writel(((MCF_BUSCLK / 16) / HZ)-1, MCF_DTIM2_DTRR);
	__raw_writew(MCF_DTIM_DTMR_ORRI | MCF_DTIM_DTMR_CLK_DIV16 |
		MCF_DTIM_DTMR_FRR  | MCF_DTIM_DTMR_RST_EN, \
		MCF_DTIM2_DTMR);

	request_irq(mcf_timervector, coldfire_dtim_clk_irq, IRQF_DISABLED, \
		"timer2", (void *)evt);

	settimericr(3, mcf_timerlevel);
	return;
}

#endif

void coldfire_reboot(void)
{
	/* disable interrupts and do a software reset */
	asm("movew #0x2700, %%sr\n\t"
	    "moveb #0x80, %%d0\n\t"
	    "moveb %%d0, 0xfc0a0000\n\t"
	    : : : "%d0");
}

static void coldfire_get_model(char *model)
{
	sprintf(model, "Version 4 ColdFire");
}

/*
 * Setup the nodes and the boot memory.
 *
 * memory_start and memory_end are VIRTUAL addresses
 */
static void __init
coldfire_bootmem_alloc(unsigned long memory_start, unsigned long memory_end)
{
	unsigned long base_pfn;

	/* compute total pages in system */
	num_pages = PAGE_ALIGN(memory_end - PAGE_OFFSET) >> PAGE_SHIFT;

	/* align start/end to page boundries */
	memory_start = PAGE_ALIGN(memory_start);
	memory_end = memory_end & PAGE_MASK;

	/* page numbers */
	base_pfn = __pa(PAGE_OFFSET) >> PAGE_SHIFT;
	min_low_pfn = __pa(memory_start) >> PAGE_SHIFT;
	max_low_pfn = __pa(memory_end) >> PAGE_SHIFT;

	high_memory = (void *)memory_end;
	availmem = memory_start;

	/* setup bootmem data */
	m68k_setup_node(0);
	availmem += init_bootmem_node(NODE_DATA(0), min_low_pfn,
		base_pfn, max_low_pfn);
	availmem = PAGE_ALIGN(availmem);

	printk(KERN_INFO "** availmem=0x%lx  pa(am)=0x%lx\n",
			availmem, __pa(availmem));
	printk(KERN_INFO "** mstart=0x%lx  mend=0x%lx\n",
			memory_start, memory_end);
	printk(KERN_INFO "bpfn=0x%lx minpfn=0x%lx maxpfn=0x%lx\n",
			base_pfn, min_low_pfn, max_low_pfn);

	/* turn over physram */
	free_bootmem(__pa(availmem), memory_end - availmem);

	/* configure physical dma area */
	cf_dma_base = __pa(PAGE_ALIGN(memory_start));
	cf_dma_size = CONFIG_DMA_SIZE;
	cf_dma_end = CONFIG_SDRAM_BASE + cf_dma_size - 1;

	printk(KERN_INFO "dma: phys base=0x%lx  phys end=0x%lx  virt base=0x%x\n",
	       cf_dma_base, cf_dma_end, CONFIG_DMA_BASE);

	printk(KERN_INFO "mdma=0x%x  pa(mdma)=0x%lx\n",
			MAX_DMA_ADDRESS, __pa(MAX_DMA_ADDRESS));
}

/*
 * Architecture setup for Coldfire
 */
void __init config_coldfire(void)
{
	unsigned long endmem, startmem;

	/*
	 * Calculate endmem from m68k_memory[0] assuming that a single
	 * chunk of ram is being configured.
	 */
	startmem = ((((int) &_end) + (PAGE_SIZE - 1)) & PAGE_MASK);
	endmem = PAGE_OFFSET + m68k_memory[0].size;

	/* set virt to node shift to be the high nibble byte */
	m68k_virt_to_node_shift = 28;

	printk(KERN_INFO "starting up linux startmem 0x%lx, endmem 0x%lx, \
		size %luMB\n", startmem,  endmem, (endmem - startmem) >> 20);

	memset(irq_enable, 0, sizeof(irq_enable));

/* JKM -- moved early*/
/*register_console(&mcfrs_console);*/

	/*
	 * Setup coldfire mach-specific handlers
	 */
	mach_max_dma_address 	= 0xffffffff;
#ifndef CONFIG_GENERIC_CLOCKEVENTS
	mach_sched_init 	= coldfire_sched_init;
	mach_tick		= coldfire_tick;
	mach_gettimeoffset 	= coldfire_gettimeoffset;
#endif
	mach_reset 		= coldfire_reboot;
/*	mach_hwclk 		= coldfire_hwclk; to be done */
	mach_get_model 		= coldfire_get_model;

	coldfire_bootmem_alloc(startmem, endmem-1);

	/*
	 * initrd setup
	 */
/* #ifdef CONFIG_BLK_DEV_INITRD
	if (m68k_ramdisk.size)  {
		reserve_bootmem (__pa(m68k_ramdisk.addr), m68k_ramdisk.size);
		initrd_start = (unsigned long) m68k_ramdisk.addr;
		initrd_end = initrd_start + m68k_ramdisk.size;
		printk (KERN_DEBUG "initrd: %08lx - %08lx\n", initrd_start,
			initrd_end);
	}
#endif */

#if defined(CONFIG_DUMMY_CONSOLE) || defined(CONFIG_FRAMEBUFFER_CONSOLE)
	conswitchp = &dummy_con;
#endif

#if defined(CONFIG_SERIAL_COLDFIRE)
	/*
	 * This causes trouble when it is re-registered later.
	 * Currently this is fixed by conditionally commenting
	 * out the register_console in mcf_serial.c
	 */
/*JKM -- moved earlier*/
	register_console(&mcfrs_console);
#endif
}

#ifdef CONFIG_SRAM
static __init void setup_sram_pool(void)
{
	declare_sram_pool((void *)CONFIG_SRAM_BASE, CONFIG_SRAM_SIZE);
}
postcore_initcall(setup_sram_pool);
#endif
