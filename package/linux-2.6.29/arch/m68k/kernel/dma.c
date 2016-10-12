/*
 * Copyright Freescale Semiconductor, Inc. 2008-2010
 *    Jason Jin Jason.Jin@freescale.com
 *    Shrek Wu B16972@freescale.com
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#undef DEBUG

#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/scatterlist.h>
#include <linux/vmalloc.h>
#include <linux/pci.h>
#include <asm/pgalloc.h>

void *dma_alloc_coherent(struct device *dev, size_t size,
			 dma_addr_t *handle, gfp_t flag)
{
#if defined(CONFIG_M5445X) || defined(CONFIG_M547X_8X) || \
    defined(CONFIG_M5441X)
        /*
	* On the M5445x platform the memory allocated with GFP_DMA
	* is guaranteed to be DMA'able.
	*/
        void *addr;

        size = PAGE_ALIGN(size);
        addr = kmalloc(size, GFP_DMA);
        *handle = virt_to_phys(addr);
        return addr;
#else
	struct page *page, **map;
	pgprot_t pgprot;
	void *addr;
	int i, order;

	pr_debug("dma_alloc_coherent: %d,%x\n", size, flag);

	size = PAGE_ALIGN(size);
	order = get_order(size);

	page = alloc_pages(flag, order);
	if (!page)
		return NULL;

	*handle = page_to_phys(page);
	map = kmalloc(sizeof(struct page *) << order, flag & ~__GFP_DMA);
	if (!map) {
		__free_pages(page, order);
		return NULL;
	}
	split_page(page, order);

	order = 1 << order;
	size >>= PAGE_SHIFT;
	map[0] = page;
	for (i = 1; i < size; i++)
		map[i] = page + i;
	for (; i < order; i++)
		__free_page(page + i);
	pgprot = __pgprot(_PAGE_PRESENT | _PAGE_ACCESSED | _PAGE_DIRTY);
	if (CPU_IS_040_OR_060)
		pgprot_val(pgprot) |= _PAGE_GLOBAL040 | _PAGE_NOCACHE_S;
	else
		pgprot_val(pgprot) |= _PAGE_NOCACHE030;
	addr = vmap(map, size, VM_MAP, pgprot);
	kfree(map);

	return addr;
#endif
}
EXPORT_SYMBOL(dma_alloc_coherent);

void dma_free_coherent(struct device *dev, size_t size,
		       void *addr, dma_addr_t handle)
{
	pr_debug("dma_free_coherent: %p, %x\n", addr, handle);
#if defined(CONFIG_M5445X) || defined(CONFIG_M547X_8X) || \
    defined(CONFIG_M5441X)
	kfree(addr);
#else
	vfree(addr);
#endif
}
EXPORT_SYMBOL(dma_free_coherent);

void dma_sync_single_for_device(struct device *dev, dma_addr_t handle,
				size_t size, enum dma_data_direction dir)
{
	switch (dir) {
	case DMA_TO_DEVICE:
		cache_push(handle, size);
		break;
	case DMA_FROM_DEVICE:
		cache_clear(handle, size);
		break;
	default:
		if (printk_ratelimit())
			printk("dma_sync_single_for_device: unsupported dir %u\n", dir);
		break;
	}
}
EXPORT_SYMBOL(dma_sync_single_for_device);

void dma_sync_sg_for_device(struct device *dev, struct scatterlist *sg, int nents,
			    enum dma_data_direction dir)
{
	int i;
#ifdef CONFIG_COLDFIRE
	struct scatterlist *_sg;

	for_each_sg(sg, _sg, nents, i)
		dma_sync_single_for_device(dev, _sg->dma_address,
				_sg->length, dir);
#else
	for (i = 0; i < nents; sg++, i++)
		dma_sync_single_for_device(dev, sg->dma_address, sg->length, dir);
#endif
}
EXPORT_SYMBOL(dma_sync_sg_for_device);

dma_addr_t dma_map_single(struct device *dev, void *addr, size_t size,
			  enum dma_data_direction dir)
{
	dma_addr_t handle = virt_to_bus(addr);

	dma_sync_single_for_device(dev, handle, size, dir);
	return handle;
}
EXPORT_SYMBOL(dma_map_single);

dma_addr_t dma_map_page(struct device *dev, struct page *page,
			unsigned long offset, size_t size,
			enum dma_data_direction dir)
{
	dma_addr_t handle = page_to_phys(page) + offset;

	dma_sync_single_for_device(dev, handle, size, dir);
	return handle;
}
EXPORT_SYMBOL(dma_map_page);

int dma_map_sg(struct device *dev, struct scatterlist *sg, int nents,
	       enum dma_data_direction dir)
{
	int i;
#ifdef CONFIG_COLDFIRE
	struct scatterlist *_sg;
#endif
#ifndef CONFIG_COLDFIRE
	for (i = 0; i < nents; sg++, i++) {
		sg->dma_address = sg_phys(sg);
		dma_sync_single_for_device(dev, sg->dma_address, sg->length, dir);
#else
	for_each_sg(sg, _sg, nents, i) {
		_sg->dma_address = sg_phys(_sg);
		dma_sync_single_for_device(dev, _sg->dma_address,
				_sg->length, dir);
#endif
	}
	return nents;
}
EXPORT_SYMBOL(dma_map_sg);
