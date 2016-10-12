/*
 * arch/m68k/coldfire/m5445x/mcf5445x-pci.c
 *
 * Coldfire M5445x specific PCI implementation.
 *
 * Copyright 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Kurt Mahan <kmahan@freescale.com>
 */

#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/pci.h>

#include <asm/mcfsim.h>
#include <asm/pci.h>
#include <asm/irq.h>
#include <asm/mcf5445x_pciarb.h>
#include <asm/mcf5445x_pci.h>
#include <asm/cf_io.h>
/*
 * Layout MCF5445x to PCI memory mappings:
 *
 *	WIN         MCF5445x                    PCI            TYPE
 *	---         --------                    ---            ----
 *     [0] 0xA0000000 -> 0xACFFFFFF  0xA0000000 -> 0xACFFFFFF  MEM
 *     [1] 0xAC000000 -> 0xAEFFFFFF  0xAC000000 -> 0xAEFFFFFF  IO
 *     [2] 0xAF000000 -> 0xAFFFFFFF  0xAF000000 -> 0xAFFFFFFF  CONFIG
 */

void __iomem *pci_mem_map;
unsigned long pci_mem_mapsize = 256 * 1024 * 1024;

#define MCF5445X_PCI_MEM_BASE		((unsigned int)pci_mem_map)
#define MCF5445X_PCI_MEM_BASE_PHY	0xA0000000
#define MCF5445X_PCI_MEM_SIZE		0x0C000000

#define MCF5445X_PCI_IO_BASE		\
	(MCF5445X_PCI_MEM_BASE + MCF5445X_PCI_MEM_SIZE)
#define MCF5445X_PCI_IO_SIZE		0x03000000

#define MCF5445X_PCI_CONFIG_BASE	\
	(MCF5445X_PCI_IO_BASE + MCF5445X_PCI_IO_SIZE)
#define MCF5445X_PCI_CONFIG_SIZE	0x01000000

#define HOST_IO_BASE			\
	(MCF5445X_PCI_MEM_BASE + MCF5445X_PCI_MEM_SIZE)
#define PCI_IO_MASK			(MCF5445X_PCI_IO_SIZE - 1)

/* #define DEBUG 1 */
#undef DEBUG
#ifdef DEBUG
# define DBG(x...) printk(x)
#else
# define DBG(x...)
#endif

/* PCI Bus memory resource block */
struct resource pci_iomem_resource = {
	.name = "PCI memory space",
	.flags = IORESOURCE_MEM,
};

/* PCI Bus ioport resource block */
struct resource pci_ioport_resource = {
	.name = "PCI I/O space",
	.flags = IORESOURCE_IO,
};

/*
 * The M54455EVB multiplexes all the PCI interrupts via
 * the FPGA and routes them to a single interrupt.  The
 * PCI spec requires all PCI interrupt routines be smart
 * enough to sort out their own interrupts.
 * The interrupt source from the FPGA is configured
 * to EPORT 3.
 */
#define MCF5445X_PCI_IRQ		0x43

#define PCI_SLOTS			4

/*
 * FPGA Info
 */
#define FPGA_PCI_IRQ_ENABLE		(u32 *)0x09000000
#define FPGA_PCI_IRQ_STATUS		(u32 *)0x09000004
#define FPGA_PCI_IRQ_ROUTE 		(u32 *)0x0900000c
#define FPGA_SEVEN_LED			(u32 *)0x09000014

extern void set_fpga(u32 *addr, u32 val);

#ifdef DEBUG
void mcf5445x_pci_dumpregs(void);
#endif

/*
 * mcf5445x_conf_device(struct pci_dev *dev)
 *
 * Machine dependent Configure the given device.
 *
 * Parameters:
 *
 * dev		- the pci device.
 */
void
mcf5445x_conf_device(struct pci_dev *dev)
{
	set_fpga(FPGA_PCI_IRQ_ENABLE, 0x0f);
}

/*
 * int mcf5445x_pci_config_read(unsigned int seg, unsigned int bus,
 *			      	unsigned int devfn, int reg,
 *			      	u32 *value)
 *
 * Read from PCI configuration space.
 *
 */
int mcf5445x_pci_config_read(unsigned int seg, unsigned int bus,
			     unsigned int devfn, int reg, int len, u32 *value)
{
	u32 addr = MCF_PCI_PCICAR_BUSNUM(bus) |
		   MCF_PCI_PCICAR_DEVNUM(PCI_SLOT(devfn)) |
		   MCF_PCI_PCICAR_FUNCNUM(PCI_FUNC(devfn)) |
		   MCF_PCI_PCICAR_DWORD(reg) |
		   MCF_PCI_PCICAR_E;

	if ((bus > 255) || (devfn > 255) || (reg > 255)) {
		*value = -1;
		return -EINVAL;
	}

	/* setup for config mode */
	MCF_PCI_PCICAR = addr;
	__asm__ __volatile__("nop");

	switch (len) {
	case 1:
		*value = *(volatile u8 *)(MCF5445X_PCI_CONFIG_BASE+(reg&3));
		break;
	case 2:
		*value = le16_to_cpu(*(volatile u16 *)
				(MCF5445X_PCI_CONFIG_BASE + (reg&2)));
		break;
	case 4:
		*value = le32_to_cpu(*(volatile u32 *)
				(MCF5445X_PCI_CONFIG_BASE));
		break;
	}

	/* clear config mode */
	MCF_PCI_PCICAR = ~MCF_PCI_PCICAR_E;
	__asm__ __volatile__("nop");

	return 0;
}

/*
 * int mcf5445x_pci_config_write(unsigned int seg, unsigned int bus,
 *			      	 unsigned int devfn, int reg,
 *			      	 u32 *value)
 *
 * Write to PCI configuration space
 */
int mcf5445x_pci_config_write(unsigned int seg, unsigned int bus,
		    unsigned int devfn, int reg, int len, u32 value)
{
	u32 addr = MCF_PCI_PCICAR_BUSNUM(bus) |
		   MCF_PCI_PCICAR_DEVNUM(PCI_SLOT(devfn)) |
		   MCF_PCI_PCICAR_FUNCNUM(PCI_FUNC(devfn)) |
		   MCF_PCI_PCICAR_DWORD(reg) |
		   MCF_PCI_PCICAR_E;

	if ((bus > 255) || (devfn > 255) || (reg > 255))
		return -EINVAL;

	/* setup for config mode */
	MCF_PCI_PCICAR = addr;
	__asm__ __volatile__("nop");

	switch (len) {
	case 1:
		*(volatile u8 *)(MCF5445X_PCI_CONFIG_BASE+(reg&3)) = (u8)value;
		break;
	case 2:
		*(volatile u16 *)(MCF5445X_PCI_CONFIG_BASE+(reg&2)) =
				cpu_to_le16((u16)value);
		break;
	case 4:
		*(volatile u32 *)(MCF5445X_PCI_CONFIG_BASE) =
				cpu_to_le32(value);
		break;
	}

	/* clear config mode */
	MCF_PCI_PCICAR = ~MCF_PCI_PCICAR_E;
	__asm__ __volatile__("nop");

	return 0;
}

/* hardware operations */
static struct pci_raw_ops mcf5445x_pci_ops = {
	.read =		mcf5445x_pci_config_read,
	.write =	mcf5445x_pci_config_write,
};

/************************************************************************/

/*
 * u8 pci_inb()
 *
 * Read a byte at specified address from I/O space
 */
unsigned char pci_inb(long addr)
{
	char value;

	value = *(volatile unsigned char *) (HOST_IO_BASE |
			(addr & PCI_IO_MASK));
	DBG("PCI: inb addr=0x%08X, value=0x%02X\n", addr, value);

	return (unsigned char) value;
}


/*
 * u16 pci_inw()
 *
 * Read a word at specified address from I/O space
 */
unsigned short pci_inw(long addr)
{
	short value;
	volatile unsigned short *ptr;

	ptr = (volatile unsigned short *) (HOST_IO_BASE | (addr & PCI_IO_MASK));
	value = le16_to_cpu(*ptr);

	DBG("PCI: inw addr=0x%08X, value=0x%04X\n",  addr, value);
	return (unsigned short) value;
}

/*
 * u16 pci_raw_inw()
 *
 * Read a raw word at specified address from I/O space
 */
unsigned short pci_raw_inw(long addr)
{
	short value;
	volatile unsigned short *ptr;

	ptr = (volatile unsigned short *) (HOST_IO_BASE | (addr & PCI_IO_MASK));
	value = *ptr;

	DBG("PCI: raw_inw addr=0x%08X, value=0x%04X\n",  addr, value);
	return (unsigned short) value;
}

/*
 * u32 pci_inl()
 *
 * Read a dword at specified address from I/O space
 */
unsigned long pci_inl(long addr)
{
	long value;
	volatile unsigned long *ptr;

	ptr = (volatile unsigned long *) (HOST_IO_BASE | (addr & PCI_IO_MASK));
	value = le32_to_cpu(*ptr);

	DBG("PCI: inl addr=0x%08X, value=0x%08X\n",  addr, value);
	return (unsigned long) value;
}

/*
 * u32 pci_raw_inl()
 *
 * Read a raw dword at specified address from I/O space
 */
unsigned long pci_raw_inl(long addr)
{
	long value;
	volatile unsigned long *ptr;

	ptr = (volatile unsigned long *) (HOST_IO_BASE | (addr & PCI_IO_MASK));
	value = *ptr;

	DBG("PCI: raw_inl addr=0x%08X, value=0x%08X\n",  addr, value);
	return (unsigned long) value;
}

/*
 * void pci_outb()
 *
 * Write a byte value at specified address to I/O space
 */
void pci_outb(unsigned char value,  long addr)
{

	*(volatile unsigned char *) (HOST_IO_BASE | (addr & PCI_IO_MASK)) \
		= value;
	DBG("PCI: outb addr=0x%08X, value=0x%02X\n",  addr, value);
}


/*
 * void pci_outw()
 *
 * Write a word value at specified address to I/O space
 */
void pci_outw(volatile unsigned short value, volatile  long addr)
{
	volatile unsigned short *ptr;

	ptr = (volatile unsigned short *) (HOST_IO_BASE | (addr & PCI_IO_MASK));
	*ptr = cpu_to_le16(value);
	DBG("PCI: outw addr=0x%08X, value=0x%04X\n",  addr, value);
}

/*
 * void pci_raw_outw()
 *
 * Write a raw word value at specified address to I/O space
 */
void pci_raw_outw(volatile unsigned short value, volatile  long addr)
{
	volatile unsigned short *ptr;

	ptr = (volatile unsigned short *) (HOST_IO_BASE | (addr & PCI_IO_MASK));
	*ptr = value;
	DBG("PCI: raw_outw addr=0x%08X, value=0x%04X\n",  addr, value);
}

/*
 * void pci_outl()
 *
 * Write a long word value at specified address to I/O space
 */
void pci_outl(volatile unsigned long value, volatile long addr)
{
	volatile unsigned long *ptr;

	ptr = (volatile unsigned long *)(HOST_IO_BASE | (addr & PCI_IO_MASK));
	*ptr = cpu_to_le32(value);
	DBG("PCI: outl addr=0x%08X, value=0x%08X\n", addr, value);
}

/*
 * void pci_raw_outl()
 *
 * Write a raw long word value at specified address to I/O space
 */
void pci_raw_outl(volatile unsigned long value, volatile long addr)
{
	volatile unsigned long *ptr;

	ptr = (volatile unsigned long *)(HOST_IO_BASE | (addr & PCI_IO_MASK));
	*ptr = value;
	DBG("PCI: raw_outl addr=0x%08X, value=0x%08X\n", addr, value);
}

/*
 * void pci_insb()
 *
 * Read several byte values from specified I/O port
 */
void pci_insb(volatile unsigned char *addr, unsigned char *buf, int len)
{
	for (; len--; buf++)
		*buf = pci_inb((unsigned long)addr);
	DBG("PCI: pci_insb addr=0x%08X, buf=%p, len=%d\n", addr, buf, len);
}


/*
 * void pci_insw()
 *
 * Read several word values from specified I/O port
 */
void pci_insw(volatile unsigned short *addr, unsigned short *buf, int len)
{
	for (; len--; buf++)
		*buf = pci_inw((unsigned long)addr);
	DBG("PCI: pci_insw addr=0x%08X, buf=%p, len=%d\n", addr, buf, len);
}

/*
 * void pci_insl()
 *
 * Read several dword values from specified I/O port
 */
void pci_insl(volatile unsigned long *addr, unsigned long *buf, int len)
{
	for (; len--; buf++)
		*buf = pci_inl((unsigned long)addr);
	DBG("PCI: pci_insl addr=0x%08X, buf=%p, len=%d\n", addr, buf, len);
}

/*
 * void pci_outsb()
 *
 * Write several byte values to specified I/O port
 */
void pci_outsb(volatile unsigned char *addr, const unsigned char *buf, int len)
{
	for (; len--; buf++)
		pci_outb((unsigned long)addr, *buf);
	DBG("PCI: pci_outsb addr=0x%08X, buf=%p, len=%d\n", addr, buf, len);
}

/*
 * void pci_outsw()
 *
 * Write several word values to specified I/O port
 */
void
pci_outsw(volatile unsigned short *addr, const unsigned short *buf, int len)
{
	for (; len--; buf++)
		pci_outw((unsigned long)addr, *buf);
	DBG("PCI: pci_outsw addr=0x%08X, buf=%p, len=%d\n", addr, buf, len);
}

/*
 * void pci_outsl()
 *
 * Write several dword values to specified I/O port
 */
void pci_outsl(volatile unsigned long *addr, const unsigned long *buf, int len)
{
	for (; len--; buf++)
		pci_outl((unsigned long)addr, *buf);
	DBG("PCI: pci_outsl addr=0x%08X, buf=%p, len=%d\n", addr, buf, len);
}
/*
 * irqreturn_t mcf5445x_pci_interrupt( int irq, void *dev)
 *
 * PCI controller interrupt handler.
 */
static irqreturn_t
mcf5445x_pci_interrupt(int irq, void *dev)
{
	u32 status = MCF_PCI_PCIGSCR;
#ifdef DEBUG
	printk(KERN_INFO "PCI: Controller irq status=0x%08x\n", status);
#endif
	/* clear */
	MCF_PCI_PCIGSCR = status;

	return IRQ_HANDLED;
}

/*
 * irqreturn_t mcf5445x_pci_arb_interrupt( int irq, void *dev)
 *
 * PCI Arbiter interrupt handler.
 */
static irqreturn_t
mcf5445x_pci_arb_interrupt(int irq, void *dev)
{
	u32 status = MCF_PCIARB_PASR;
#ifdef DEBUG
	printk(KERN_INFO "PCI: Arbiter irq status=0x%08x\n", status);
#endif
	/* clear */
	MCF_PCIARB_PASR = status;
	return IRQ_HANDLED;
}

/*
 * struct pci_bus_info *init_mcf5445x_pci(void)
 *
 * Machine specific initialisation:
 *
 * - Allocate and initialise a 'pci_bus_info' structure
 * - Initialize hardware
 *
 * Result: pointer to 'pci_bus_info' structure.
 */
int __init
init_mcf5445x_pci(void)
{
	/*
	 * Initialize the PCI core
	 */
	printk(KERN_INFO "init_mcf5445x_pci\n");

	/* pci memory mapping */
	pci_mem_map = ioremap(MCF5445X_PCI_MEM_BASE_PHY, pci_mem_mapsize);
	if (!pci_mem_map) {
		printk(KERN_ERR "PCI memory map failed by ioremap!\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "MCF5445X_PCI_MEM_BASE value is 0x%x\n", \
                           MCF5445X_PCI_MEM_BASE);

	/* Initialize pci resource */
	pci_iomem_resource.start = MCF5445X_PCI_MEM_BASE;
	pci_iomem_resource.end = MCF5445X_PCI_MEM_BASE + \
                                 MCF5445X_PCI_MEM_SIZE - 1;

	pci_ioport_resource.start = MCF5445X_PCI_IO_BASE;
	pci_ioport_resource.end = MCF5445X_PCI_IO_BASE + \
                                  MCF5445X_PCI_IO_SIZE - 1;

	/*
	* Must Reset!!! If bootloader has PCI enabled, it will cause
	* problem in linux when it tries to configure/find resources
	* for the pci devices
	*/
	MCF_PCI_PCIGSCR = 1;

	/* arbitration controller */
	MCF_PCIARB_PACR = MCF_PCIARB_PACR_INTMPRI |
			  MCF_PCIARB_PACR_EXTMPRI(0x0f) |
			  MCF_PCIARB_PACR_INTMINTEN |
			  MCF_PCIARB_PACR_EXTMINTEN(0x0f);

	/* pci pin assignment regs */
#if defined(CONFIG_PATA_FSL) || defined(CONFIG_PATA_FSL_MODULE)
	MCF_GPIO_PAR_PCI = MCF_GPIO_PAR_PCI_GNT0 |
			MCF_GPIO_PAR_PCI_GNT1 |
			MCF_GPIO_PAR_PCI_GNT2 |
			MCF_GPIO_PAR_PCI_GNT3_GNT3 |
			MCF_GPIO_PAR_PCI_REQ0 |
			MCF_GPIO_PAR_PCI_REQ1 |
			MCF_GPIO_PAR_PCI_REQ2 |
			MCF_GPIO_PAR_PCI_REQ3_REQ3;

	MCF_GPIO_PAR_PCI = (MCF_GPIO_PAR_PCI &
			(MCF_GPIO_PAR_PCI_GNT3_MASK &
			 MCF_GPIO_PAR_PCI_REQ3_MASK)) |
			MCF_GPIO_PAR_PCI_GNT3_ATA_DMACK |
			MCF_GPIO_PAR_PCI_REQ3_ATA_INTRQ;
#else
	MCF_GPIO_PAR_PCI = MCF_GPIO_PAR_PCI_GNT0 |
			   MCF_GPIO_PAR_PCI_GNT1 |
			   MCF_GPIO_PAR_PCI_GNT2 |
			   MCF_GPIO_PAR_PCI_GNT3_GNT3 |
			   MCF_GPIO_PAR_PCI_REQ0 |
			   MCF_GPIO_PAR_PCI_REQ1 |
			   MCF_GPIO_PAR_PCI_REQ2 |
			   MCF_GPIO_PAR_PCI_REQ3_REQ3;
#endif
	/* target control reg */
	MCF_PCI_PCITCR = MCF_PCI_PCITCR_P |
			 MCF_PCI_PCITCR_WCT(8);

	/* PCI MEM address */
	MCF_PCI_PCIIW0BTAR = MCF5445X_PCI_MEM_BASE_PHY |
			     (MCF5445X_PCI_MEM_BASE >> 16);

	/* PCI MEM address */
	MCF_PCI_PCIIW1BTAR = (MCF5445X_PCI_MEM_BASE_PHY +
                             MCF5445X_PCI_MEM_SIZE)
			     | (MCF5445X_PCI_IO_BASE >> 16);

	/* PCI IO address */
	MCF_PCI_PCIIW2BTAR = (MCF5445X_PCI_MEM_BASE_PHY +
                             MCF5445X_PCI_MEM_SIZE + MCF5445X_PCI_IO_SIZE)
			     | (MCF5445X_PCI_CONFIG_BASE >> 16);

	/* window control */
	MCF_PCI_PCIIWCR = MCF_PCI_PCIIWCR_WINCTRL0_ENABLE |
			  MCF_PCI_PCIIWCR_WINCTRL0_MEMREAD |
			  MCF_PCI_PCIIWCR_WINCTRL1_ENABLE |
			  MCF_PCI_PCIIWCR_WINCTRL1_IO |
			  MCF_PCI_PCIIWCR_WINCTRL2_ENABLE |
			  MCF_PCI_PCIIWCR_WINCTRL2_IO;

	/* initiator control reg */
	MCF_PCI_PCIICR = 0;

	/* type 0 - command */
	MCF_PCI_PCISCR = MCF_PCI_PCISCR_MW |	/* mem write/inval */
			 MCF_PCI_PCISCR_B |	/* bus master enable */
			 MCF_PCI_PCISCR_MA |	/* clear master abort error */
			 MCF_PCI_PCISCR_M;	/* mem access enable */

	/* type 0 - config reg */
	MCF_PCI_PCICR1 = MCF_PCI_PCICR1_CACHELINESIZE(4) |
			 MCF_PCI_PCICR1_LATTIMER(0xFF);
	/* type 0 - config 2 reg */
	MCF_PCI_PCICR2 = 0;

	/* target control reg */
	MCF_PCI_PCITCR2 = MCF_PCI_PCITCR2_B0E | MCF_PCI_PCITCR2_B5E;

	/* translate addresses from PCI[0] to CF[SDRAM] */
	MCF_PCI_PCITBATR0 = 0xFC000000 | MCF_PCI_PCITBATR5_EN;
	MCF_PCI_PCITBATR5 = MCF_RAMBAR1 | MCF_PCI_PCITBATR5_EN;

	/* inbound window for memory */
	MCF_PCI_PCIBAR0 = 0xFC000000;
	MCF_PCI_PCIBAR5 = MCF_RAMBAR1;

	/* setup controller interrupt handlers */
	if (request_irq(55+128, mcf5445x_pci_interrupt, IRQF_SHARED,
			"PCI Controller", NULL))
		printk(KERN_ERR "PCI: Unable to register controller irq\n");

	if (request_irq(56+128, mcf5445x_pci_arb_interrupt, IRQF_SHARED,
			"PCI Arbiter", NULL))
		printk(KERN_ERR "PCI: Unable to register arbiter irq\n");

	/* global control - clear reset bit */
	MCF_PCI_PCIGSCR = MCF_PCI_PCIGSCR_SEE |
			  MCF_PCI_PCIGSCR_PEE;

	/* let everything settle */
	udelay(1000);

	/* allocate bus ioport resource */
	if (request_resource(&ioport_resource, &pci_ioport_resource) < 0)
		printk(KERN_ERR "PCI: Unable to alloc ioport resource\n");

	/* allocate bus iomem resource */
	if (request_resource(&iomem_resource, &pci_iomem_resource) < 0)
		printk(KERN_ERR "PCI: Unable to alloc iomem resource\n");

	/* setup FPGA to route PCI to IRQ3(67), SW7 to IRQ7, SW6 to IRQ4 */
	set_fpga(FPGA_PCI_IRQ_ENABLE, 0x00000000);
	set_fpga(FPGA_PCI_IRQ_ROUTE, 0x00000039);
	set_fpga(FPGA_SEVEN_LED, 0x000000FF);

	raw_pci_ops = &mcf5445x_pci_ops;

	return 0;
}

/*
 * DEBUGGING
 */

#ifdef DEBUG
struct regdump {
	u32 addr;
	char regname[16];
};

struct regdump type0regs[] = {
	{ 0xfc0a8000, "PCIIDR" },
	{ 0xfc0a8004, "PCISCR" },
	{ 0xfc0a8008, "PCICCRIR" },
	{ 0xfc0a800c, "PCICR1" },
	{ 0xfc0a8010, "PCIBAR0" },
	{ 0xfc0a8014, "PCIBAR1" },
	{ 0xfc0a8018, "PCIBAR2" },
	{ 0xfc0a801c, "PCIBAR3" },
	{ 0xfc0a8020, "PCIBAR4" },
	{ 0xfc0a8024, "PCIBAR5" },
	{ 0xfc0a8028, "PCICCPR" },
	{ 0xfc0a802c, "PCISID" },
	{ 0xfc0a8030, "PCIERBAR" },
	{ 0xfc0a8034, "PCICPR" },
	{ 0xfc0a803c, "PCICR2" },
	{ 0, "" }
};

struct regdump genregs[] = {
	{ 0xfc0a8060, "PCIGSCR" },
	{ 0xfc0a8064, "PCITBATR0" },
	{ 0xfc0a8068, "PCITBATR1" },
	{ 0xfc0a806c, "PCITCR1" },
	{ 0xfc0a8070, "PCIIW0BTAR" },
	{ 0xfc0a8074, "PCIIW1BTAR" },
	{ 0xfc0a8078, "PCIIW2BTAR" },
	{ 0xfc0a8080, "PCIIWCR" },
	{ 0xfc0a8084, "PCIICR" },
	{ 0xfc0a8088, "PCIISR" },
	{ 0xfc0a808c, "PCITCR2" },
	{ 0xfc0a8090, "PCITBATR0" },
	{ 0xfc0a8094, "PCITBATR1" },
	{ 0xfc0a8098, "PCITBATR2" },
	{ 0xfc0a809c, "PCITBATR3" },
	{ 0xfc0a80a0, "PCITBATR4" },
	{ 0xfc0a80a4, "PCITBATR5" },
	{ 0xfc0a80a8, "PCIINTR" },
	{ 0xfc0a80f8, "PCICAR" },
	{ 0, "" }
};

struct regdump arbregs[] = {
	{ 0xfc0ac000, "PACR" },
	{ 0xfc0ac004, "PASR" },	/* documentation error */
	{ 0, "" }
};

/*
 * void mcf5445x_pci_dumpregs()
 *
 * Dump out all the PCI registers
 */
void
mcf5445x_pci_dumpregs(void)
{
	struct regdump *reg;

	printk(KERN_INFO "*** MCF5445x PCI TARGET 0 REGISTERS ***\n");

	reg = type0regs;
	while (reg->addr) {
		printk(KERN_INFO "0x%08x  0x%08x  %s\n", reg->addr,
			*((u32 *)reg->addr), reg->regname);
		reg++;
	}

	printk(KERN_INFO "\n*** MCF5445x PCI GENERAL REGISTERS ***\n");
	reg = genregs;
	while (reg->addr) {
		printk(KERN_INFO "0x%08x  0x%08x  %s\n", reg->addr,
			*((u32 *)reg->addr), reg->regname);
		reg++;
	}
	printk(KERN_INFO "\n*** MCF5445x PCI ARBITER REGISTERS ***\n");
	reg = arbregs;
	while (reg->addr) {
		printk(KERN_INFO "0x%08x  0x%08x  %s\n", reg->addr,
			*((u32 *)reg->addr), reg->regname);
		reg++;
	}
}
#endif /* DEBUG */
