/*
 * asm-m68k/pci.h - m68k specific PCI declarations.
 *
 * Copyright 2007, 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Kurt Mahan <kmahan@freescale.com>
 */
#ifndef _ASM_M68K_5445X_PCI_H
#define _ASM_M68K_5445x_PCI_H

#ifndef CONFIG_PCI
/*
 * The PCI address space does equal the physical memory
 * address space.  The networking and block device layers use
 * this boolean for bounce buffer decisions.
 */
#define PCI_DMA_BUS_IS_PHYS		(1)
#else
#include <asm-generic/pci-dma-compat.h>

#define PCI_DMA_BASE                    0 /* PCI-DMA window base */
#define NL_ORIGINAL
/*
 * The PCI address space does equal the physical memory
 * address space.  The networking and block device layers use
 * this boolean for bounce buffer decisions.
 */
#define PCI_DMA_BUS_IS_PHYS		(1)

#define PCIBIOS_MIN_IO			0x00004000
#define PCIBIOS_MIN_MEM			0x02000000

#define pcibios_assign_all_busses()	0
#define pcibios_scan_all_fns(a, b)	0

struct pci_raw_ops {
	int (*read)(unsigned int domain, unsigned int bus, unsigned int devfn,
			int reg, int len, u32 *val);
	int (*write)(unsigned int domain, unsigned int bus, unsigned int devfn,
			int reg, int len, u32 val);
};

extern struct pci_raw_ops *raw_pci_ops;

static inline void
pcibios_set_master(struct pci_dev *dev)
{
	/* no special bus mastering setup handling */
}

static inline void
pcibios_penalize_isa_irq(int irq, int active)
{
	/* no dynamic PCI IRQ allocation */
}

#if 0
static inline void
pcibios_add_platform_entries(struct pci_dev *dev)
{
	/* no special handling */
}
#endif

static inline void
pcibios_resource_to_bus(struct pci_dev *dev, struct pci_bus_region *region,
			 struct resource *res)
{
	region->start = res->start;
	region->end = res->end;
}

static inline void
pcibios_bus_to_resource(struct pci_dev *dev, struct resource *res,
			struct pci_bus_region *region)
{
	res->start = region->start;
	res->end = region->end;
}

static inline struct resource *
pcibios_select_root(struct pci_dev *pdev, struct resource *res)
{
	struct resource *root = NULL;

	if (res->flags & IORESOURCE_IO)
		root = &ioport_resource;
	if (res->flags & IORESOURCE_MEM)
		root = &iomem_resource;

	return root;
}

void
pcibios_align_resource(void *data, struct resource *res, resource_size_t size,
		       resource_size_t align);

#endif /* CONFIG_PCI */
#endif /* _ASM_M68K_5445X_PCI_H */
