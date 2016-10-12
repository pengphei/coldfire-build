/*
 * linux/sound/coldfire/snd-coldfire.c
 *
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Author: York Sun <r58495@freescale.com>
 *         Alison Wang <b18965@freescale.com>
 *
 * Coldfire ALSA driver based on SSI and TLV320A
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************
 *
 * NOTE: This driver was tested on MCF5445x, MCF5301x, MCF5227x, MCF532x and
 * MCF537x platforms.
 * */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/ioctl.h>
#include <linux/soundcard.h>
#include <linux/spi/spi.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/control.h>
#include <linux/fs.h>
#include <linux/delay.h>

#include <asm/mcfsim.h>
#include <asm/coldfire.h>
#include <asm/mcf_edma.h>
#include "snd-coldfire.h"

#if defined(CONFIG_M5445X)
#include <asm/mcf5445x_ssi.h>
#endif

#define CF_ALSA_DEBUG 0
#if CF_ALSA_DEBUG
#define DBG(fmt, args...) 	printk(KERN_INFO "[%s]  " fmt , \
				__func__, ## args)
#else
#define DBG(fmt, args...)	do {} while (0)
#endif

#define SOUND_CARD_NAME		"Coldfire ALSA"
#define MAX_BUFFER_SIZE		(32*1024)

/* eDMA channel for SSI channel 0,1 TX,RX */
#define DMA_TX_TCD0	MCF_EDMA_CHAN_TIMER2
#define DMA_TX_TCD1	MCF_EDMA_CHAN_TIMER3
#define DMA_RX_TCD0	MCF_EDMA_CHAN_TIMER0
#define DMA_RX_TCD1	MCF_EDMA_CHAN_TIMER1

#define CODEC_LEFT_IN_REG		(0x00)
#define CODEC_RIGHT_IN_REG		(0x01)
#define CODEC_LEFT_HP_VOL_REG		(0x02)
#define CODEC_RIGHT_HP_VOL_REG		(0x03)
#define CODEC_ANALOG_APATH_REG		(0x04)
#define CODEC_DIGITAL_APATH_REG		(0x05)
#define CODEC_POWER_DOWN_REG		(0x06)
#define CODEC_DIGITAL_IF_FMT_REG	(0x07)
#define CODEC_SAMPLE_RATE_REG		(0x08)
#define CODEC_DIGITAL_IF_ACT_REG	(0x09)
#define CODEC_RESET_REG			(0x0f)

#define TLV320AIC23_CODEC_SAMPLE_RATE_REG	(0x08)
#define TLV320AIC23_CODEC_SAMPLERATE_REG_8KHZ	(0x0C)
#define TLV320AIC23_CODEC_SAMPLERATE_REG_11KHZ	(0x0C)
#define TLV320AIC23_CODEC_SAMPLERATE_REG_16KHZ	(0x58)
#define TLV320AIC23_CODEC_SAMPLERATE_REG_22KHZ 	(0x62)
#define TLV320AIC23_CODEC_SAMPLERATE_REG_44KHZ 	(0x22)
#define TLV320AIC23_CODEC_SAMPLERATE_REG_48KHZ 	(0x00)

#define MCF_SSI_AUDIO_IRQ_LEVEL		(5)
#define TLV320A_VOL_MAX			0x07F
#define TLV320A_VOL_INIT		0x065
#define TLV320A_LINEIN_MAX		0x1F
#define TLV320A_LINEIN_INIT		0x17
#define TLV320A_ANALOGPATH_INIT		0x11

/* Codec settings */
#define MCF_SSI_AUDIO_MCLK_1	(12288000U) /*Hz*/
#define MCF_SSI_AUDIO_MCLK_2	(16934400U) /*Hz*/
#define MCF_SSI_AUDIO_MCLK_3	(14112000U) /*Hz*/
#define MCF_SSI_AUDIO_MCLK_4    (18432000U) /*Hz*/

#define MCF_SSI_AUDIO_SSDIV_VALUE_1	\
	((((u32)MCF_CLK*2)/MCF_SSI_AUDIO_MCLK_1)+ \
	(((((u32)MCF_CLK*2*10)/MCF_SSI_AUDIO_MCLK_1)%10) > 5))

#define MCF_SSI_AUDIO_SSDIV_VALUE_2	\
	((((u32)MCF_CLK*2)/MCF_SSI_AUDIO_MCLK_2)+ \
	(((((u32)MCF_CLK*2*10)/MCF_SSI_AUDIO_MCLK_2)%10) > 5))

#define MCF_SSI_AUDIO_SSDIV_VALUE_3	\
	((((u32)MCF_CLK*2)/MCF_SSI_AUDIO_MCLK_3)+ \
	(((((u32)MCF_CLK*2*10)/MCF_SSI_AUDIO_MCLK_3)%10) > 5))

#define MCF_SSI_AUDIO_SSDIV_VALUE_4	\
	((((u32)MCF_CLK*2)/MCF_SSI_AUDIO_MCLK_4)+ \
	(((((u32)MCF_CLK*2*10)/MCF_SSI_AUDIO_MCLK_4)%10) > 5))

#define SNDRV_COLDFIRE_PCM_PLAYBACK_FORMATS	SNDRV_PCM_FMTBIT_S16_BE
#define SNDRV_COLDFIRE_PCM_CAPTURE_FORMATS	SNDRV_PCM_FMTBIT_S16_BE

#define RXFWM	2
#define TXFWM	2
#define HW_PERIODS_BYTES_MIN	4096
#define HW_PERIODS_BYTES_STEP	4096

#define INPUT_MICROPHONE 0
#define INPUT_LINEIN 1
#define NUM_TCDS 4

static char *id;
static struct platform_device *device;
static int g_tx_dmaing;
static int g_rx_dmaing;
static unsigned char g_mastervol, g_lineinvol, g_analogpath;

/** Use 4 TCDs for scatter/gather address
 * to setup dma chain, one TCD per period
 * so that we don't need change them on the fly
 */

/**
 *  Link Descriptor
 *
 *  must be aligned on a 32-byte boundary.
 */
struct dma_tcd {
	__be32 saddr;		/* source address */
	__be16 attr;		/* transfer attribute */
	__be16 soffset;		/* source offset */
	__be32 nbytes;		/* minor byte count */
	__be32 slast;		/* last source address adjust */
	__be32 daddr;		/* dest address */
	__be16 citer;		/* current minor looplink, major count */
	__be16 doffset;		/* dest offset */
	__be32 dlast_sga;	/* last dest addr adjust, scatter/gather addr*/
	__be16 biter;		/* begging minor looklink, major count */
	__be16 csr;		/* control and status */
} __attribute__ ((aligned(32), packed));

/** dma_private: p-substream DMA data
 *
 * The tcd[] array is first because it needs to be aligned on a 32-byte
 * boundary, so putting it first will ensure alignment without padding the
 * structure.
 *
 * @tcd[]: array of TCDs
 */
struct dma_private {
	struct dma_tcd tcd0[NUM_TCDS];
	struct dma_tcd tcd1[NUM_TCDS];
	dma_addr_t tcd_buf_phys;	/* physical address of dma_private */
	dma_addr_t dma_buf_phys;
	dma_addr_t dma_buf_next;
	dma_addr_t dma_buf_end;
	size_t period_size;
	unsigned int num_periods;
};

struct tlv320a_audio_device {
	struct spi_device *spi;
	u32 speed;
	u32 stereo;
	u32 bits;
	u32 format;
	u8 isopen;
	u8 dmaing;
	u8 ssi_enabled;
	u8 channel;
	spinlock_t lock;
	u8 *audio_buf;
};

/* chip specific define */
struct chip_spec {
	struct snd_card *card;
	struct snd_pcm *pcm;
	struct tlv320a_audio_device *audio_device;
	u32 offset;
	void *mixer_data;
};

/* hardware definition */
static struct snd_pcm_hardware snd_coldfire_playback_hw = {
	.info = (SNDRV_PCM_INFO_INTERLEAVED |
#if defined(CONFIG_MMU)
		SNDRV_PCM_INFO_MMAP |
		SNDRV_PCM_INFO_MMAP_VALID|
#endif
		SNDRV_PCM_INFO_BLOCK_TRANSFER),
	.formats = SNDRV_COLDFIRE_PCM_PLAYBACK_FORMATS,
	.rates = SNDRV_PCM_RATE_8000_48000,
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 2,
	.buffer_bytes_max = MAX_BUFFER_SIZE,
	.period_bytes_min = HW_PERIODS_BYTES_MIN,
	.period_bytes_max = MAX_BUFFER_SIZE/NUM_TCDS,
	.periods_min = NUM_TCDS,
	.periods_max = NUM_TCDS,
	.fifo_size = 0,
};

/* hardware definition */
static struct snd_pcm_hardware snd_coldfire_capture_hw = {
	.info = (SNDRV_PCM_INFO_INTERLEAVED |
#if defined(CONFIG_MMU)
		SNDRV_PCM_INFO_MMAP |
		SNDRV_PCM_INFO_MMAP_VALID|
#endif
		SNDRV_PCM_INFO_BLOCK_TRANSFER),
	.formats = SNDRV_COLDFIRE_PCM_CAPTURE_FORMATS,
	.rates = SNDRV_PCM_RATE_8000_48000,
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 2,
	.buffer_bytes_max = MAX_BUFFER_SIZE,
	.period_bytes_min = HW_PERIODS_BYTES_MIN,
	.period_bytes_max = MAX_BUFFER_SIZE/NUM_TCDS,
	.periods_min = NUM_TCDS,
	.periods_max = NUM_TCDS,
	.fifo_size = 0,
};

static unsigned int rates[] = {8000,  11025, 16000, 22000,
			       22050, 44000, 44100, 48000};

/* hw constraints */
static struct snd_pcm_hw_constraint_list constraints_rates = {
	.count = ARRAY_SIZE(rates),
	.list = rates,
	.mask = 0,
};

static inline void ssi_audio_dma_playback_start(void)
{
	g_tx_dmaing = 1;
	mcf_edma_start_transfer(DMA_TX_TCD0);
	mcf_edma_start_transfer(DMA_TX_TCD1);
}

static inline void ssi_audio_dma_capture_start(void)
{
	g_rx_dmaing = 1;
	mcf_edma_start_transfer(DMA_RX_TCD0);
	mcf_edma_start_transfer(DMA_RX_TCD1);
}

static inline void ssi_audio_dma_playback_stop(void)
{
	g_tx_dmaing = 0;
	mcf_edma_stop_transfer(DMA_TX_TCD0);
	mcf_edma_stop_transfer(DMA_TX_TCD1);
}

inline void ssi_audio_dma_capture_stop(void)
{
	g_rx_dmaing = 0;
	mcf_edma_stop_transfer(DMA_RX_TCD0);
	mcf_edma_stop_transfer(DMA_RX_TCD1);
}

/**
 * fill_tcd_params - Fill transfer control descriptor (TCD)
 * @base: base address of TCD
 * @source: source address
 * @dest: destination address
 * @attr: attributes
 * @soff: source offset
 * @nbytes: number of bytes to be transfered in minor loop
 * @slast: last source address adjustment
 * @citer: major loop count
 * @biter: beginning minor loop count
 * @doff: destination offset
 * @dlast_sga: last destination address adjustment
 * @major_int: generate interrupt after each major loop
 * @disable_req: disable DMA request after major loop
 * @enable_sg: enable scatter/gather address
 */
void fill_tcd_params(u32 base, u32 source, u32 dest,
			u32 attr, u32 soff, u32 nbytes, u32 slast,
			u32 citer, u32 biter, u32 doff, u32 dlast_sga,
			int major_int, int disable_req, int enable_sg)
{
	struct dma_tcd *tcd = (struct dma_tcd *) base;

	tcd->saddr = source;
	tcd->attr = attr;
	tcd->soffset = soff;
	tcd->nbytes = nbytes;
	tcd->slast = slast;
	tcd->daddr = dest;
	tcd->citer = citer & 0x7fff;
	tcd->doffset = doff;
	tcd->dlast_sga = dlast_sga;
	tcd->biter = biter & 0x7fff;
	tcd->csr = ((major_int) ? 0x2 : 0)	|
		   ((disable_req) ? 0x8 : 0)	|
		   ((enable_sg) ? 0x10 : 0);
}

static int
ssi_audio_dma_playback_config(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct dma_private *dma_private = runtime->private_data;
	u32 size = frames_to_bytes(runtime, runtime->period_size);
	u32 offset, soffset, daddr0, daddr1, attr, sga0, sga1;
	u32 i, nbyte, major_loops;

	if ((runtime->channels < 1) || (runtime->channels > 2)) {
		printk(KERN_ERR "Error on channels = %d\n", runtime->channels);
		return -EINVAL;
	}

	dma_private->dma_buf_phys = runtime->dma_addr;
	dma_private->dma_buf_next = dma_private->dma_buf_phys;
	dma_private->dma_buf_end = dma_private->dma_buf_phys +
					runtime->periods * size;

	if (runtime->format == SNDRV_PCM_FORMAT_S16_BE) {
		nbyte = 2 * TXFWM;
		soffset = 2 * runtime->channels;
		daddr0 = (u32)&MCF_SSI_TX0 + 2;
		daddr1 = (u32)&MCF_SSI_TX1 + 2;
		attr = 	MCF_EDMA_TCD_ATTR_SSIZE_16BIT |
			MCF_EDMA_TCD_ATTR_DSIZE_16BIT;
	} else {
		printk(KERN_ERR "Not supported PCM format %x\n",
		       runtime->format);
		return -EINVAL;
	}

	major_loops = size/nbyte/runtime->channels;
	sga0 = (u32)dma_private->tcd_buf_phys;
	sga1 = (u32)dma_private->tcd_buf_phys +
		4 * sizeof(struct dma_tcd);

#if defined(CONFIG_M5301x) || defined(CONFIG_M5445X)
	MCF_EDMA_TCD10_CSR = 0;
	MCF_EDMA_TCD11_CSR = 0;
#else
	MCF_EDMA_TCD11_CSR = 0;
	MCF_EDMA_TCD12_CSR = 0;
#endif
	offset = (runtime->channels - 1) * 2;
	mcf_edma_set_tcd_params(DMA_TX_TCD0,
			(u32)dma_private->dma_buf_next,
			daddr0,
			attr,
			soffset,
			nbyte,
			0,		/* slast */
			major_loops,	/* citer */
			major_loops,	/* biter */
			0,		/* dest offset */
			sga0,
			1,		/* major_int */
			0);		/* enable dma request after */

	mcf_edma_set_tcd_params(DMA_TX_TCD1,
			(u32)dma_private->dma_buf_next + offset,
			daddr1,
			attr,
			soffset,
			nbyte,
			0,		/* slast */
			major_loops,	/* citer */
			major_loops,	/* biter */
			0,		/* dest offset */
			sga1,
			0,		/* major_int */
			0);		/* enable dma request after */

	while (!(MCF_EDMA_TCD_CSR(DMA_TX_TCD0) & MCF_EDMA_TCD_CSR_E_SG))
		MCF_EDMA_TCD_CSR(DMA_TX_TCD0) |= MCF_EDMA_TCD_CSR_E_SG;
	while (!(MCF_EDMA_TCD_CSR(DMA_TX_TCD1) & MCF_EDMA_TCD_CSR_E_SG))
		MCF_EDMA_TCD_CSR(DMA_TX_TCD1) |= MCF_EDMA_TCD_CSR_E_SG;

	for (i = 0; i < NUM_TCDS; i++) {
		dma_private->dma_buf_next += size;
		if (dma_private->dma_buf_next >= dma_private->dma_buf_end)
			dma_private->dma_buf_next = dma_private->dma_buf_phys;
		sga0 = (u32)dma_private->tcd_buf_phys +
			((i+1)%NUM_TCDS) * sizeof(struct dma_tcd);
		sga1 = (u32)dma_private->tcd_buf_phys +
			((i+1)%NUM_TCDS + 4) * sizeof(struct dma_tcd);
		DBG("sga0 = 0x%x, sga1 = 0x%x.\n", sga0, sga1);
		fill_tcd_params((u32)&dma_private->tcd0[i],
				(u32)dma_private->dma_buf_next,
				daddr0,
				attr,
				soffset,
				nbyte,
				0,		/* slast */
				major_loops,	/* citer */
				major_loops,	/* biter */
				0,		/* dest offset */
				sga0,
				1,		/* major_int */
				0,		/* enable dma request after */
				1);		/* enable scatter/gather */

		fill_tcd_params((u32)&dma_private->tcd1[i],
				(u32)dma_private->dma_buf_next + offset,
				daddr1,
				attr,
				soffset,
				nbyte,
				0,		/* slast */
				major_loops,	/* citer */
				major_loops,	/* biter */
				0,		/* dest offset */
				sga1,
				0,		/* major_int */
				0,		/* enable dma request after */
				1);		/* enable scatter/gather */
	}

	return 0;
}

static int
ssi_audio_dma_capture_config(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct dma_private *dma_private = runtime->private_data;
	u32 size = frames_to_bytes(runtime, runtime->period_size);
	u32 offset, saddr0, saddr1, doffset, attr, sga0, sga1;
	int i, nbyte, major_loops;

	if ((runtime->channels < 1) || (runtime->channels > 2)) {
		printk(KERN_ERR "Error on channels = %d\n", runtime->channels);
		return -EINVAL;
	}

	dma_private->dma_buf_phys = runtime->dma_addr;
	dma_private->dma_buf_next = dma_private->dma_buf_phys;
	dma_private->dma_buf_end = dma_private->dma_buf_phys +
					runtime->periods * size;

	switch (runtime->format) {
	case SNDRV_PCM_FORMAT_S16_BE:
		saddr0 = (u32)&MCF_SSI_RX0 + 2;
		saddr1 = (u32)&MCF_SSI_RX1 + 2;
		nbyte = 2 * RXFWM;
		doffset = 2 * runtime->channels;
		attr = 	MCF_EDMA_TCD_ATTR_SSIZE_16BIT |
			MCF_EDMA_TCD_ATTR_DSIZE_16BIT;
		break;
	default:
		printk(KERN_ERR "Not supported PCM format %x\n",
		       runtime->format);
		return -EINVAL;
	}

	major_loops = size/nbyte/runtime->channels;
	sga0 = (u32)dma_private->tcd_buf_phys;
	sga1 = (u32)dma_private->tcd_buf_phys +
		4 * sizeof(struct dma_tcd);

#if defined(CONFIG_M5301x) || defined(CONFIG_M5445X)
	MCF_EDMA_TCD8_CSR = 0;
	MCF_EDMA_TCD9_CSR = 0;
#else
	MCF_EDMA_TCD9_CSR = 0;
	MCF_EDMA_TCD10_CSR = 0;
#endif
	offset = (runtime->channels - 1) * 2;
	mcf_edma_set_tcd_params(DMA_RX_TCD0,
			saddr0,
			(u32)dma_private->dma_buf_next,
			attr,
			0,		/* source offset */
			nbyte,
			0,		/* slast */
			major_loops,	/* citer */
			major_loops,	/* biter */
			doffset,
			sga0,
			1,		/* major_int */
			0);		/* enable dma request after */

	mcf_edma_set_tcd_params(DMA_RX_TCD1,
			saddr1,
			(u32)dma_private->dma_buf_next + offset,
			attr,
			0,		/* source offset */
			nbyte,
			0,		/* slast */
			major_loops,	/* citer */
			major_loops,	/* biter */
			doffset,
			sga1,
			0,		/* major_int */
			0);		/* enable dma request after */

	while (!(MCF_EDMA_TCD_CSR(DMA_RX_TCD0) & MCF_EDMA_TCD_CSR_E_SG))
		MCF_EDMA_TCD_CSR(DMA_RX_TCD0) |= MCF_EDMA_TCD_CSR_E_SG;
	while (!(MCF_EDMA_TCD_CSR(DMA_RX_TCD1) & MCF_EDMA_TCD_CSR_E_SG))
		MCF_EDMA_TCD_CSR(DMA_RX_TCD1) |= MCF_EDMA_TCD_CSR_E_SG;

	for (i = 0; i < NUM_TCDS; i++) {
		dma_private->dma_buf_next += size;
		if (dma_private->dma_buf_next >= dma_private->dma_buf_end)
			dma_private->dma_buf_next = dma_private->dma_buf_phys;
		sga0 = (u32)dma_private->tcd_buf_phys +
			((i+1)%NUM_TCDS) * sizeof(struct dma_tcd);
		sga1 = (u32)dma_private->tcd_buf_phys +
			((i+1)%NUM_TCDS + 4) * sizeof(struct dma_tcd);
		fill_tcd_params((u32)&dma_private->tcd0[i],
				saddr0,
				(u32)dma_private->dma_buf_next,
				attr,
				0,		/* source offset */
				nbyte,
				0,		/* slast */
				major_loops,	/* citer */
				major_loops,	/* biter */
				doffset,
				sga0,
				1,		/* major_int */
				0,		/* enable dma request after */
				1);		/* enable scatter/gather */
		fill_tcd_params((u32)&dma_private->tcd1[i],
				saddr1,
				(u32)dma_private->dma_buf_next + offset,
				attr,
				0,		/* source offset */
				nbyte,
				0,		/* slast */
				major_loops,	/* citer */
				major_loops,	/* biter */
				doffset,
				sga1,
				0,		/* major_int */
				0,		/* enable dma request after */
				1);		/* enable scatter/gather */
	}
	return 0;
}

static inline void ssi_audio_enable_ssi_playback(void)
{
	MCF_SSI_CR |= MCF_SSI_CR_SSI_EN | MCF_SSI_CR_TE;
}

static inline void ssi_audio_enable_ssi_capture(void)
{
	MCF_SSI_CR |= MCF_SSI_CR_SSI_EN | MCF_SSI_CR_RE;
}

static inline void ssi_audio_disable_ssi(void)
{
	MCF_SSI_CR &= ~(MCF_SSI_CR_TE | MCF_SSI_CR_RE | MCF_SSI_CR_SSI_EN);
}

static inline void ssi_audio_disable_ssi_playback(void)
{
	MCF_SSI_CR &= ~MCF_SSI_CR_TE;
}

static inline void ssi_audio_disable_ssi_capture(void)
{
	MCF_SSI_CR &= ~MCF_SSI_CR_RE;
}

static int ssi_audio_dma_playback_handler(int channel, void *dev_id)
{
	struct snd_pcm_substream *substream;
	struct snd_pcm_runtime *runtime;

	substream = (struct snd_pcm_substream *)dev_id;
	runtime = substream->runtime;

	/* inform ALSA middle layer about transfer status */
	snd_pcm_period_elapsed(substream);
	mcf_edma_confirm_interrupt_handled(DMA_TX_TCD0);
	mcf_edma_confirm_interrupt_handled(DMA_TX_TCD1);

	return IRQ_HANDLED;
}

static int ssi_audio_dma_capture_handler(int channel, void *dev_id)
{
	struct snd_pcm_substream *substream;
	struct snd_pcm_runtime *runtime;

	substream = (struct snd_pcm_substream *)dev_id;
	runtime = substream->runtime;

	/* inform ALSA middle layer about transfer status */
	snd_pcm_period_elapsed(substream);
	mcf_edma_confirm_interrupt_handled(DMA_RX_TCD0);
	mcf_edma_confirm_interrupt_handled(DMA_RX_TCD1);

	return IRQ_HANDLED;
}

int ssi_audio_dma_request_playback_channel(struct snd_pcm_substream *substream)
{
	int err;
	struct chip_spec *chip = snd_pcm_substream_chip(substream);

	/* request eDMA channel */
	err = mcf_edma_request_channel(DMA_TX_TCD0,
				ssi_audio_dma_playback_handler,
				NULL,
				MCF_SSI_AUDIO_IRQ_LEVEL,
				substream,
				&(chip->audio_device->lock),
				id);
	if (err < 0)
		return err;
	err = mcf_edma_request_channel(DMA_TX_TCD1,
				ssi_audio_dma_playback_handler,
				NULL,
				MCF_SSI_AUDIO_IRQ_LEVEL,
				substream,
				&(chip->audio_device->lock),
				id);
	return err;
}

int ssi_audio_dma_request_capture_channel(struct snd_pcm_substream *substream)
{
	int err;
	struct chip_spec *chip = snd_pcm_substream_chip(substream);

	/* request 2 eDMA channels for two fifo */
	err = mcf_edma_request_channel(DMA_RX_TCD0,
				ssi_audio_dma_capture_handler,
				NULL,
				MCF_SSI_AUDIO_IRQ_LEVEL,
				substream,
				&(chip->audio_device->lock),
				id);
	if (err < 0)
		return err;
	err = mcf_edma_request_channel(DMA_RX_TCD1,
				ssi_audio_dma_capture_handler,
				NULL,
				MCF_SSI_AUDIO_IRQ_LEVEL,
				substream,
				&(chip->audio_device->lock),
				id);
	return err;
}

static inline void ssi_audio_init_dma(void)
{
	/* SSI DMA Signals mapped to DMA request */
	MCF_CCM_MISCCR &= ~MCF_CCM_MISCCR_TIM_DMA;
}

static void ssi_audio_adjust_codec_speed(struct snd_pcm_substream *substream)
{
	ssi_audio_disable_ssi();

	if (substream->runtime->format == SNDRV_PCM_FORMAT_S16_BE) {
		MCF_SSI_CCR = MCF_SSI_CCR_WL(7) |	/* 16 bit word length */
				MCF_SSI_CCR_DC(1);	/* Frame rate divider */
	}

	switch (substream->runtime->rate) {
	case 8000:
#if defined(CONFIG_M532x) || defined(CONFIG_M537x)
		MCF_CCM_CDR = (MCF_CCM_CDR | MCF_CCM_CDR_SSIDIV(0x20))
			| MCF_CCM_CDR_SSIDIV(MCF_SSI_AUDIO_SSDIV_VALUE_1);
#else
		MCF_CCM_CDR = (MCF_CCM_CDR & ~MCF_CCM_CDR_SSIDIV(0xFF)) |
			MCF_CCM_CDR_SSIDIV(MCF_SSI_AUDIO_SSDIV_VALUE_1);
#endif
		MCF_SSI_CCR |= MCF_SSI_CCR_PM(11);
		mcf_codec_spi_write(TLV320AIC23_CODEC_SAMPLE_RATE_REG,
				TLV320AIC23_CODEC_SAMPLERATE_REG_8KHZ);
		break;
	case 11000:
	case 11025:
#if defined(CONFIG_M532x) || defined(CONFIG_M537x)
		MCF_CCM_CDR = (MCF_CCM_CDR & ~MCF_CCM_CDR_SSIDIV(0x3F)) |
			MCF_CCM_CDR_SSIDIV(0x2B);
		MCF_SSI_CCR |= MCF_SSI_CCR_PM(11);
#else
		MCF_CCM_CDR = (MCF_CCM_CDR & ~MCF_CCM_CDR_SSIDIV(0xFF)) |
			MCF_CCM_CDR_SSIDIV(MCF_SSI_AUDIO_SSDIV_VALUE_3);
		MCF_SSI_CCR |= MCF_SSI_CCR_PM(9);
#endif
		mcf_codec_spi_write(TLV320AIC23_CODEC_SAMPLE_RATE_REG,
				TLV320AIC23_CODEC_SAMPLERATE_REG_11KHZ);
		break;
	case 16000:
#if defined(CONFIG_M532x) || defined(CONFIG_M537x)
		MCF_CCM_CDR = (MCF_CCM_CDR | MCF_CCM_CDR_SSIDIV(0x20))
			| MCF_CCM_CDR_SSIDIV(MCF_SSI_AUDIO_SSDIV_VALUE_1);
#else
		MCF_CCM_CDR = (MCF_CCM_CDR & ~MCF_CCM_CDR_SSIDIV(0xFF)) |
			MCF_CCM_CDR_SSIDIV(MCF_SSI_AUDIO_SSDIV_VALUE_1);
#endif
		MCF_SSI_CCR |= MCF_SSI_CCR_PM(5);
		mcf_codec_spi_write(TLV320AIC23_CODEC_SAMPLE_RATE_REG,
				TLV320AIC23_CODEC_SAMPLERATE_REG_16KHZ);
		break;
	case 22000:
	case 22050:
#if defined(CONFIG_M532x) || defined(CONFIG_M537x)
		MCF_CCM_CDR = (MCF_CCM_CDR & ~MCF_CCM_CDR_SSIDIV(0x3F)) |
			MCF_CCM_CDR_SSIDIV(0x2B);
		MCF_SSI_CCR |= MCF_SSI_CCR_PM(5);
#else
		MCF_CCM_CDR = (MCF_CCM_CDR & ~MCF_CCM_CDR_SSIDIV(0xFF)) |
			MCF_CCM_CDR_SSIDIV(MCF_SSI_AUDIO_SSDIV_VALUE_3);
		MCF_SSI_CCR |= MCF_SSI_CCR_PM(4);
#endif
		mcf_codec_spi_write(TLV320AIC23_CODEC_SAMPLE_RATE_REG,
				TLV320AIC23_CODEC_SAMPLERATE_REG_22KHZ);
		break;
	case 48000:
#if defined(CONFIG_M532x) || defined(CONFIG_M537x)
		MCF_CCM_CDR = (MCF_CCM_CDR & ~MCF_CCM_CDR_SSIDIV(0x3F)) |
			MCF_CCM_CDR_SSIDIV(MCF_SSI_AUDIO_SSDIV_VALUE_4);
		MCF_SSI_CCR |= MCF_SSI_CCR_PM(3);
#else
		MCF_CCM_CDR = (MCF_CCM_CDR & ~MCF_CCM_CDR_SSIDIV(0xFF)) |
			MCF_CCM_CDR_SSIDIV(MCF_SSI_AUDIO_SSDIV_VALUE_1);
		MCF_SSI_CCR |= MCF_SSI_CCR_PM(1);
#endif
		mcf_codec_spi_write(TLV320AIC23_CODEC_SAMPLE_RATE_REG,
				TLV320AIC23_CODEC_SAMPLERATE_REG_48KHZ);
		break;
	case 44000:
	case 44100:
	default:
#if defined(CONFIG_M532x) || defined(CONFIG_M537x)
		MCF_CCM_CDR = (MCF_CCM_CDR & ~MCF_CCM_CDR_SSIDIV(0x3F)) |
			MCF_CCM_CDR_SSIDIV(0x2B);
#else
		MCF_CCM_CDR = (MCF_CCM_CDR & ~MCF_CCM_CDR_SSIDIV(0xFF)) |
			MCF_CCM_CDR_SSIDIV(MCF_SSI_AUDIO_SSDIV_VALUE_2);
#endif
		MCF_SSI_CCR |= MCF_SSI_CCR_PM(2);
		mcf_codec_spi_write(TLV320AIC23_CODEC_SAMPLE_RATE_REG,
			TLV320AIC23_CODEC_SAMPLERATE_REG_44KHZ);
		break;
	}
	DBG("MCF_CCM_CDR = 0x%x, MCF_SSI_CCR = 0x%x.\n",
			MCF_CCM_CDR, MCF_SSI_CCR);
}

static void ssi_audio_codec_reset(void)
{
	mcf_codec_spi_write(CODEC_RESET_REG, 0); /* reset the audio chip */
	udelay(2500); /* wait for reset */
}

static void
ssi_audio_init_codec_for_playback(struct snd_pcm_substream *substream)
{
	mcf_codec_spi_write(CODEC_LEFT_IN_REG, g_lineinvol);
	mcf_codec_spi_write(CODEC_RIGHT_IN_REG, g_lineinvol);
	mcf_codec_spi_write(CODEC_POWER_DOWN_REG, 0x060);
	mcf_codec_spi_write(CODEC_DIGITAL_IF_FMT_REG, 0x02);
	mcf_codec_spi_write(CODEC_DIGITAL_APATH_REG, 0x006);
	mcf_codec_spi_write(CODEC_DIGITAL_IF_ACT_REG, 0x001);
	mcf_codec_spi_write(CODEC_ANALOG_APATH_REG, g_analogpath);
	mcf_codec_spi_write(CODEC_LEFT_HP_VOL_REG, g_mastervol);
	mcf_codec_spi_write(CODEC_RIGHT_HP_VOL_REG, g_mastervol);
}

static void
ssi_audio_init_codec_for_capture(struct snd_pcm_substream *substream)
{
	mcf_codec_spi_write(CODEC_LEFT_IN_REG, g_lineinvol);
	mcf_codec_spi_write(CODEC_RIGHT_IN_REG, g_lineinvol);
	mcf_codec_spi_write(CODEC_POWER_DOWN_REG, 0x060);
	mcf_codec_spi_write(CODEC_DIGITAL_IF_FMT_REG, 0x02);
	mcf_codec_spi_write(CODEC_DIGITAL_APATH_REG, 0x006);
	mcf_codec_spi_write(CODEC_DIGITAL_IF_ACT_REG, 0x001);
	mcf_codec_spi_write(CODEC_ANALOG_APATH_REG, g_analogpath);
	mcf_codec_spi_write(CODEC_LEFT_HP_VOL_REG, g_mastervol);
	mcf_codec_spi_write(CODEC_RIGHT_HP_VOL_REG, g_mastervol);
}

static void ssi_audio_chip_init(void)
{
	int chip_initialized = 0;
	if (chip_initialized == 1)
		return;

	ssi_audio_init_dma();
	/* Enable the SSI pins */
#if defined(CONFIG_M5227x)
	MCF_GPIO_PAR_UART = (MCF_GPIO_PAR_UART
			&~MCF_GPIO_PAR_UART_PAR_U1TXD(0xF)
			&~MCF_GPIO_PAR_UART_PAR_U1RXD(0xF)
			&~MCF_GPIO_PAR_UART_PAR_U1RTS(0xF)
			&~MCF_GPIO_PAR_UART_PAR_U1CTS(0xF))
			| MCF_GPIO_PAR_UART_PAR_U1CTS_SSI_BCLK
			| MCF_GPIO_PAR_UART_PAR_U1RTS_SSI_FS
			| MCF_GPIO_PAR_UART_PAR_U1RXD_SSI_RXD
			| MCF_GPIO_PAR_UART_PAR_U1TXD_SSI_TXD;

	MCF_GPIO_PAR_TIMER = (MCF_GPIO_PAR_TIMER
			&~MCF_GPIO_PAR_TIMER_PAR_T3IN(0xF))
			| MCF_GPIO_PAR_TIMER_PAR_T3IN_SSI_MCLK;
#endif
#if defined(CONFIG_M532x)
	MCF_GPIO_PAR_SSI = (0
			| MCF_GPIO_PAR_SSI_PAR_MCLK
			| MCF_GPIO_PAR_SSI_PAR_TXD(3)
			| MCF_GPIO_PAR_SSI_PAR_RXD(3)
			| MCF_GPIO_PAR_SSI_PAR_FS(3)
			| MCF_GPIO_PAR_SSI_PAR_BCLK(3));
#endif
#if defined(CONFIG_M537x)
	MCF_GPIO_PAR_UART = (MCF_GPIO_PAR_UART
			&~MCF_GPIO_PAR_UART_PAR_UTXD1(0xF)
			&~MCF_GPIO_PAR_UART_PAR_URXD1(0xF)
			&~MCF_GPIO_PAR_UART_PAR_URTS1(0xF)
			&~MCF_GPIO_PAR_UART_PAR_UCTS1(0xF))
			| MCF_GPIO_PAR_UART_PAR_UCTS1_SSI_BCLK
			| MCF_GPIO_PAR_UART_PAR_URTS1_SSI_FS
			| MCF_GPIO_PAR_UART_PAR_URXD1_SSI_RXD
			| MCF_GPIO_PAR_UART_PAR_UTXD1_SSI_TXD;

	MCF_GPIO_PAR_IRQ = MCF_GPIO_PAR_IRQ_PAR_IRQ4(1);
#endif
#if defined(CONFIG_M5301x)
	MCF_GPIO_PAR_SSIH = (MCF_GPIO_PAR_SSIH_PAR_RXD_SSI_RXD |
			MCF_GPIO_PAR_SSIH_PAR_TXD_SSI_TXD |
			MCF_GPIO_PAR_SSIH_PAR_FS_SSI_FS |
			MCF_GPIO_PAR_SSIH_PAR_MCLK_SSI_MCLK);
	MCF_GPIO_PAR_SSIL = MCF_GPIO_PAR_SSIL_PAR_BCLK_SSI_BCLK;
#endif
#if defined(CONFIG_M5445X)
	MCF_GPIO_PAR_SSI = (MCF_GPIO_PAR_SSI_MCLK |
			MCF_GPIO_PAR_SSI_STXD_STXD |
			MCF_GPIO_PAR_SSI_SRXD_SRXD |
			MCF_GPIO_PAR_SSI_FS_FS |
			MCF_GPIO_PAR_SSI_BCLK_BCLK);
#endif
	chip_initialized = 1;
}

static void ssi_audio_init_ssi_playback(void)
{
	/* Issue a SSI reset */
	MCF_SSI_CR &= ~MCF_SSI_CR_SSI_EN;

	/* SSI module uses internal CPU clock */
	MCF_CCM_MISCCR |= MCF_CCM_MISCCR_SSI_SRC;
#if defined(CONFIG_M5445X) || defined(CONFIG_M532x) || defined(CONFIG_M537x) \
	|| defined(CONFIG_M5227x)
	MCF_CCM_MISCCR |= MCF_CCM_MISCCR_SSI_PUE | MCF_CCM_MISCCR_SSI_PUS;
#endif
#if defined(CONFIG_M5301x)
	MCF_GPIO_PCRH |= MCF_GPIO_PCRH_SSI_PUS | MCF_GPIO_PCRH_SSI_PUE;
#endif
	MCF_SSI_CR = MCF_SSI_CR_CIS |
			MCF_SSI_CR_TCH |	/* Enable two channel mode */
			MCF_SSI_CR_MCE |	/* clock out on SSI_MCLK pin */
			MCF_SSI_CR_I2S_MASTER |	/* I2S master mode */
			MCF_SSI_CR_SYN |	/* Enable synchronous mode */
			MCF_SSI_CR_NET;		/* Auto set by I2S Master */

	MCF_SSI_TCR = 0 |
			/* internally generated bit clock */
			MCF_SSI_TCR_TXDIR |
			/* internally generated frame sync */
			MCF_SSI_TCR_TFDIR |
			/* Clock data on falling edge of bit clock */
			MCF_SSI_TCR_TSCKP |
			/* Frame sync active low */
			MCF_SSI_TCR_TFSI |
			/* TX frame sync 1 bit before data */
			MCF_SSI_TCR_TEFS |
			/* TX FIFO 0 enabled */
			MCF_SSI_TCR_TFEN0 |
			/* TX FIFO 1 enabled */
			MCF_SSI_TCR_TFEN1 |
			MCF_SSI_TCR_TXBIT0;

	MCF_SSI_FCSR = MCF_SSI_FCSR_TFWM0(TXFWM) | MCF_SSI_FCSR_TFWM1(TXFWM);

	MCF_SSI_IER =	MCF_SSI_IER_TDMAE |	/* DMA request enabled */
			MCF_SSI_IER_TFE0 |
			MCF_SSI_IER_TFE1;	/* set by reset actually*/
}

static void ssi_audio_init_ssi_capture(void)
{
	/* Issue a SSI reset */
	MCF_SSI_CR &= ~MCF_SSI_CR_SSI_EN;

	/* SSI module uses internal CPU clock */
	MCF_CCM_MISCCR |= MCF_CCM_MISCCR_SSI_SRC;
#if defined(CONFIG_M5445X) || defined(CONFIG_M532x) || defined(CONFIG_M537x) \
	|| defined(CONFIG_M5227x)
	MCF_CCM_MISCCR |= MCF_CCM_MISCCR_SSI_PUE | MCF_CCM_MISCCR_SSI_PUS;
#endif
#if defined(CONFIG_M5301x)
	MCF_GPIO_PCRH |= MCF_GPIO_PCRH_SSI_PUS | MCF_GPIO_PCRH_SSI_PUE;
#endif
	MCF_SSI_CR = MCF_SSI_CR_CIS |
			MCF_SSI_CR_TCH |	/* Enable two channel mode */
			MCF_SSI_CR_MCE |	/* clock out on SSI_MCLK pin */
			MCF_SSI_CR_I2S_MASTER |	/* I2S master mode */
			MCF_SSI_CR_SYN |	/* Enable synchronous mode */
			MCF_SSI_CR_NET;		/* Auto set by I2S Master */

	MCF_SSI_TCR = 0 |
			/* internally generated bit clock */
			MCF_SSI_TCR_TXDIR |
			/* internally generated frame sync */
			MCF_SSI_TCR_TFDIR |
			/* Clock data on falling edge of bit clock */
			MCF_SSI_TCR_TSCKP |
			/* Frame sync active low */
			MCF_SSI_TCR_TFSI |
			/* TX frame sync 1 bit before data */
			MCF_SSI_TCR_TEFS |
			/* TX FIFO 0 enabled */
			MCF_SSI_TCR_TFEN0 |
			/* TX FIFO 1 enabled */
			MCF_SSI_TCR_TFEN1 |
			MCF_SSI_TCR_TXBIT0;

	MCF_SSI_RCR = 0 |
			/* Clock data on rising edge of bit clock */
			MCF_SSI_RCR_RSCKP |
			/* Frame sync active low */
			MCF_SSI_RCR_RFSI |
			/* RX frame sync 1 bit before data */
			MCF_SSI_RCR_REFS |
			/* RX FIFO 0 enabled */
			MCF_SSI_RCR_RFEN0 |
			/* RX FIFO 1 enabled */
			MCF_SSI_RCR_RFEN1 |
			MCF_SSI_RCR_RXBIT0;	/* Auto set by I2S Master */

	MCF_SSI_FCSR = MCF_SSI_FCSR_RFWM0(RXFWM) | MCF_SSI_FCSR_RFWM1(RXFWM);

	/* interrupts */
	MCF_SSI_IER =	MCF_SSI_IER_RDMAE |	/* DMA request enabled */
			MCF_SSI_IER_RFF0 |	/* rx FIFO 0 full */
			MCF_SSI_IER_RFF1;	/* rx FIFO 1 full */
}

static int snd_coldfire_playback_open(struct snd_pcm_substream *substream)
{
	int err;
	struct chip_spec *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct dma_private *dma_private;
	dma_addr_t tcd_buf_phys;

	runtime->hw = snd_coldfire_playback_hw;
	err = snd_pcm_hw_constraint_integer(runtime,
		SNDRV_PCM_HW_PARAM_PERIODS);
	if (err < 0) {
		printk(KERN_ERR "invalid buffer size\n");
		return err;
	}
	/* to make sure period_bytes is the multiple of size of minor loops */
	err = snd_pcm_hw_constraint_step(runtime, 0,
				SNDRV_PCM_HW_PARAM_PERIOD_BYTES,
				HW_PERIODS_BYTES_STEP);
	if (err < 0) {
		printk(KERN_ERR "Error setting period_bytes step, "
				"err=%d\n", err);
		return err;
	}
	err = snd_pcm_hw_constraint_list(substream->runtime, 0,
				SNDRV_PCM_HW_PARAM_RATE,
				&constraints_rates);
	if (err < 0) {
		printk(KERN_ERR "Error setting rate constraints, "
				"err=%d\n", err);
		return err;
	}
	ssi_audio_chip_init();
	ssi_audio_init_ssi_playback();
	ssi_audio_init_codec_for_playback(substream);
	err = ssi_audio_dma_request_playback_channel(substream);
	if (err < 0) {
		printk(KERN_ERR "Error requesting dma channel, err=%d\n", err);
		return err;
	}

	dma_private = dma_alloc_coherent(substream->pcm->dev,
		sizeof(struct dma_private), &tcd_buf_phys, GFP_KERNEL);

	if (!dma_private) {
		dev_err(substream->pcm->card->dev,
			"can't allocate DMA private data\n");
		return -ENOMEM;
	}

	dma_private->tcd_buf_phys = tcd_buf_phys;
	runtime->private_data = dma_private;

	chip->offset = 0;
	g_tx_dmaing = 0;
	return 0;
}

static int snd_coldfire_capture_open(struct snd_pcm_substream *substream)
{
	int err;
	struct chip_spec *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct dma_private *dma_private;
	dma_addr_t tcd_buf_phys;

	runtime->hw = snd_coldfire_capture_hw;

	err = snd_pcm_hw_constraint_integer(runtime,
		SNDRV_PCM_HW_PARAM_PERIODS);
	if (err < 0) {
		printk(KERN_ERR "invalid buffer size\n");
		return err;
	}
	/* to make sure period_bytes is the multiple of size of minor loops */
	err = snd_pcm_hw_constraint_step(runtime, 0,
				SNDRV_PCM_HW_PARAM_PERIOD_BYTES,
				HW_PERIODS_BYTES_STEP);
	if (err < 0) {
		printk(KERN_ERR "Error setting period_bytes step, "
				"err=%d\n", err);
		return err;
	}
	err = snd_pcm_hw_constraint_list(substream->runtime, 0,
				SNDRV_PCM_HW_PARAM_RATE,
				&constraints_rates);
	if (err < 0) {
		printk(KERN_ERR "Error setting pcm_hw_constraint_list, "
				"err=%d\n", err);
		return err;
	}

	ssi_audio_chip_init();
	ssi_audio_init_ssi_capture();
	ssi_audio_init_codec_for_capture(substream);
	err = ssi_audio_dma_request_capture_channel(substream);
	if (err < 0) {
		printk(KERN_ERR "Error requesting dma channel, err=%d\n", err);
		return err;
	}

	dma_private = dma_alloc_coherent(substream->pcm->dev,
		sizeof(struct dma_private), &tcd_buf_phys, GFP_KERNEL);

	if (!dma_private) {
		dev_err(substream->pcm->card->dev,
			"can't allocate DMA private data\n");
		return -ENOMEM;
	}
	dma_private->tcd_buf_phys = tcd_buf_phys;
	dma_private->dma_buf_phys = substream->dma_buffer.addr;

	runtime->private_data = dma_private;

	chip->offset = 0;
	g_rx_dmaing = 0;
	return 0;
}

static int snd_coldfire_playback_close(struct snd_pcm_substream *substream)
{
	struct dma_private *dma_private = substream->runtime->private_data;

	ssi_audio_dma_playback_stop();
	mcf_edma_free_channel(DMA_TX_TCD0, substream);
	mcf_edma_free_channel(DMA_TX_TCD1, substream);
	if (dma_private) {
		dma_free_coherent(substream->pcm->dev,
			sizeof(struct dma_private),
			dma_private, dma_private->tcd_buf_phys);
		substream->runtime->private_data = NULL;
	}
	ssi_audio_disable_ssi();
	return 0;
}

static int snd_coldfire_capture_close(struct snd_pcm_substream *substream)
{
	struct dma_private *dma_private = substream->runtime->private_data;

	ssi_audio_dma_capture_stop();
	mcf_edma_free_channel(DMA_RX_TCD0, substream);
	mcf_edma_free_channel(DMA_RX_TCD1, substream);
	/* Deallocate the fsl_dma_private structure */
	if (dma_private) {
		dma_free_coherent(substream->pcm->dev,
			sizeof(struct dma_private),
			dma_private, dma_private->tcd_buf_phys);
		substream->runtime->private_data = NULL;
	}
	ssi_audio_disable_ssi();
	return 0;
}

static int snd_coldfire_pcm_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *hw_params)
{
	int err;

	/* set runtime buffer */
	err = snd_pcm_lib_malloc_pages(
			substream, params_buffer_bytes(hw_params));
	if (err < 0)
		printk(KERN_ERR "Error allocating pages, err=%d\n", err);
	return err;
}

static int snd_coldfire_pcm_hw_free(struct snd_pcm_substream *substream)
{
	/* free the memory if was newly allocated */
	return snd_pcm_lib_free_pages(substream);
}

static int
snd_coldfire_pcm_playback_prepare(struct snd_pcm_substream *substream)
{
	int err;

	if (g_tx_dmaing == 1)
		return 0;

	ssi_audio_adjust_codec_speed(substream);
	err = ssi_audio_dma_playback_config(substream);
	if (err < 0) {
		printk(KERN_ERR "Error configuring playback, "
				"err=%d\n", err);
		return err;
	}

	ssi_audio_dma_playback_start();
	return 0;
}

static int snd_coldfire_pcm_capture_prepare(struct snd_pcm_substream *substream)
{
	int err;

	if (g_rx_dmaing == 1)
		return 0;

	ssi_audio_adjust_codec_speed(substream);
	err = ssi_audio_dma_capture_config(substream);
	if (err < 0) {
		printk(KERN_ERR "Error configuring capture, "
				"err=%d\n", err);
		return err;
	}
	ssi_audio_dma_capture_start();

	return 0;
}

static int
snd_coldfire_pcm_playback_trigger(struct snd_pcm_substream *substream, int cmd)
{
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		ssi_audio_enable_ssi_playback();
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		ssi_audio_disable_ssi_playback();
		break;
	default:
		printk(KERN_ERR "Unsupported trigger command, cmd=%d\n", cmd);
		return -EINVAL;
	}

	return 0;
}

static int
snd_coldfire_pcm_capture_trigger(struct snd_pcm_substream *substream, int cmd)
{
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		ssi_audio_enable_ssi_capture();
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		ssi_audio_disable_ssi_capture();
		break;
	default:
		printk(KERN_ERR "Unsupported trigger command, cmd=%d\n", cmd);
		return -EINVAL;
	}

	return 0;
}

static snd_pcm_uframes_t
snd_coldfire_pcm_playback_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct dma_private *dma_private = runtime->private_data;
	snd_pcm_uframes_t pointer;
	u32 offset;

	offset = (u32)(MCF_EDMA_TCD_SADDR(DMA_TX_TCD0) -
		       dma_private->dma_buf_phys);
	if (runtime->format == SNDRV_PCM_FORMAT_S16_BE)
		pointer = offset / (runtime->channels == 1 ? 2 : 4);
	else
		pointer = 0;

	return pointer;
}

static snd_pcm_uframes_t
snd_coldfire_pcm_capture_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct dma_private *dma_private = runtime->private_data;
	snd_pcm_uframes_t pointer;
	u32 offset;

	offset = (u32)(MCF_EDMA_TCD_DADDR(DMA_RX_TCD0) -
		       dma_private->dma_buf_phys);
	if (runtime->format == SNDRV_PCM_FORMAT_S16_BE)
		pointer = offset / (runtime->channels == 1 ? 2 : 4);
	else
		pointer = 0;

	return pointer;
}

static struct snd_pcm_ops snd_coldfire_playback_ops = {
	.open = snd_coldfire_playback_open,
	.close = snd_coldfire_playback_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = snd_coldfire_pcm_hw_params,
	.hw_free = snd_coldfire_pcm_hw_free,
	.prepare = snd_coldfire_pcm_playback_prepare,
	.trigger = snd_coldfire_pcm_playback_trigger,
	.pointer = snd_coldfire_pcm_playback_pointer,
};

static struct snd_pcm_ops snd_coldfire_capture_ops = {
	.open = snd_coldfire_capture_open,
	.close = snd_coldfire_capture_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = snd_coldfire_pcm_hw_params,
	.hw_free = snd_coldfire_pcm_hw_free,
	.prepare = snd_coldfire_pcm_capture_prepare,
	.trigger = snd_coldfire_pcm_capture_trigger,
	.pointer = snd_coldfire_pcm_capture_pointer,
};

static int __init snd_coldfire_new_pcm(struct chip_spec *chip)
{
	struct snd_pcm *pcm;
	int err;

	err = snd_pcm_new(chip->card, "coldfire", 0, 1, 1,
					&pcm);
	if (err < 0) {
		printk(KERN_ERR "Error creating new pcm, err=%d\n", err);
		return err;
	}
	pcm->private_data = chip;
	strncpy(pcm->name, SOUND_CARD_NAME, sizeof(pcm->name));
	chip->pcm = pcm;
	pcm->info_flags = 0;

	/* set operators */
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK,
			&snd_coldfire_playback_ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE,
			&snd_coldfire_capture_ops);
	/* pre-allocation of buffers */
	err = snd_pcm_lib_preallocate_pages_for_all(
			pcm,
			SNDRV_DMA_TYPE_CONTINUOUS,
			snd_dma_continuous_data(GFP_KERNEL),
			MAX_BUFFER_SIZE,
			MAX_BUFFER_SIZE);

	if (!pcm->streams[0].substream->dma_buffer.addr)
		pcm->streams[0].substream->dma_buffer.addr =
		virt_to_phys(pcm->streams[0].substream->dma_buffer.area);
	if (!pcm->streams[1].substream->dma_buffer.addr)
		pcm->streams[1].substream->dma_buffer.addr =
		virt_to_phys(pcm->streams[1].substream->dma_buffer.area);

	if (err) {
		printk(KERN_ERR
			"Can't pre-allocate DMA buffer (size=%u)\n",
			MAX_BUFFER_SIZE);
		return -ENOMEM;
	}

	chip->audio_device =
		kmalloc(sizeof(struct tlv320a_audio_device), GFP_DMA);

	if (!chip->audio_device) {
		snd_pcm_lib_preallocate_free_for_all(pcm);
		printk(KERN_ERR
			"Can't allocate buffer for audio device\n");
		return -ENOMEM;
	}

	return 0;
}

static int tlv320a_set_out_volume(unsigned char value)
{
	unsigned char data;

	if (value > TLV320A_VOL_MAX)
		data = TLV320A_VOL_MAX;
	else
		data = value;

	if (mcf_codec_spi_write(CODEC_LEFT_HP_VOL_REG, data) < 0)
		return -EINVAL;

	if (mcf_codec_spi_write(CODEC_RIGHT_HP_VOL_REG, data) < 0)
		return -EINVAL;

	return 0;
}

static int tlv320a_info_out_volume(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = TLV320A_VOL_MAX;
	return 0;
}

static int tlv320a_get_out_volume(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = g_mastervol;
	return 0;
}

static int tlv320a_put_out_volume(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	unsigned char vol;
	int change;

	vol = ucontrol->value.integer.value[0];

	if (vol > TLV320A_VOL_MAX)
		return -EINVAL;

	change = (g_mastervol != vol);
	if (change) {
		g_mastervol = vol;
		tlv320a_set_out_volume(vol);
	}
	return change;
}

static int tlv320a_set_linein_volume(unsigned char value)
{
	unsigned char data;

	if (value > TLV320A_LINEIN_MAX)
		data = TLV320A_LINEIN_MAX;
	else
		data = value;

	if (mcf_codec_spi_write(CODEC_LEFT_IN_REG, data) < 0)
		return -EINVAL;

	if (mcf_codec_spi_write(CODEC_RIGHT_IN_REG, data) < 0)
		return -EINVAL;

	return 0;
}

static int tlv320a_info_linein_volume(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = TLV320A_LINEIN_MAX;
	return 0;
}

static int tlv320a_get_linein_volume(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = g_lineinvol;
	return 0;
}

static int tlv320a_put_linein_volume(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	unsigned char vol;
	int change;

	vol = ucontrol->value.integer.value[0];

	if (vol > TLV320A_LINEIN_MAX)
		return -EINVAL;

	change = (g_lineinvol != vol);
	if (change) {
		g_lineinvol = vol;
		tlv320a_set_linein_volume(vol);
	}
	return change;
}

#define tlv320a_info_mic_boost	snd_ctl_boolean_mono_info
static int tlv320a_get_mic_boost(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = ((g_analogpath & 0x1) == 1);
	return 0;
}

static int tlv320a_put_mic_boost(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	int oldboost, newboost;
	u8 data;
	if (ucontrol->value.integer.value[0] == 1)
		newboost = 1;
	else
		newboost = 0;
	oldboost = g_analogpath & 0x1;

	if (oldboost == newboost)
		return 0;
	data = (g_analogpath & 0xfe) | (newboost & 0x1);
	if (mcf_codec_spi_write(CODEC_ANALOG_APATH_REG, data) < 0)
		return -EINVAL;
	g_analogpath = data;
	return 1;
}

static int tlv320a_info_capture_source(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_info *uinfo)
{
	static char *texts[] = { "Line-In", "Microphone" };

	uinfo->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
	uinfo->count = 1;
	uinfo->value.enumerated.items = 2;
	if (uinfo->value.enumerated.item > 1)
		uinfo->value.enumerated.item = 1;
	strcpy(uinfo->value.enumerated.name,
	       texts[uinfo->value.enumerated.item]);
	return 0;
}

static int tlv320a_get_capture_source(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{

	ucontrol->value.enumerated.item[0] = ((g_analogpath & 0x4) == 0x4);
	return 0;
}

static int tlv320a_put_capture_source(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	int oldinput, newinput;
	u8 data;

	if (ucontrol->value.enumerated.item[0] > 1)
		return -EINVAL;

	oldinput = (g_analogpath & 0x4) ?  INPUT_MICROPHONE : INPUT_LINEIN;

	if (ucontrol->value.enumerated.item[0])
		newinput = INPUT_MICROPHONE;
	else
		newinput = INPUT_LINEIN;
	if (oldinput == newinput)
		return 0;
	data = (g_analogpath & 0xfb) |
		(newinput == INPUT_MICROPHONE ? 0x4 : 0);
	if (mcf_codec_spi_write(CODEC_ANALOG_APATH_REG, data) < 0)
		return -EINVAL;
	g_analogpath = data;
	return 1;
}

static struct snd_kcontrol_new tlv320_mixer_out __devinitdata = {
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "Master Playback Volume",
	.index = 0,
	.access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.info = tlv320a_info_out_volume,
	.get = tlv320a_get_out_volume,
	.put = tlv320a_put_out_volume,
};

static struct snd_kcontrol_new tlv320_mixer_linein __devinitdata = {
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "Line Capture Volume",
	.access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.info = tlv320a_info_linein_volume,
	.get = tlv320a_get_linein_volume,
	.put = tlv320a_put_linein_volume,
};

static struct snd_kcontrol_new tlv320_mixer_capture_source __devinitdata = {
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "Capture source",
	.access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.info = tlv320a_info_capture_source,
	.get = tlv320a_get_capture_source,
	.put = tlv320a_put_capture_source,
};

static struct snd_kcontrol_new tlv320_mixer_mic_boost __devinitdata = {
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "Mic Boost",
	.access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.info = tlv320a_info_mic_boost,
	.get = tlv320a_get_mic_boost,
	.put = tlv320a_put_mic_boost,
};

static int __init coldfire_alsa_audio_probe(struct platform_device *dev)
{
	struct snd_card *card;
	struct chip_spec *chip;
	int err;

	card = snd_card_new(-1, id, THIS_MODULE, sizeof(struct chip_spec));
	if (card == NULL)
		return -ENOMEM;

	chip = card->private_data;

	chip->card = card;
	card->dev = &dev->dev;

	err = snd_coldfire_new_pcm(chip);
	if (err < 0)
		return -ENOMEM;

	strcpy(card->driver, "coldfire");
	strcpy(card->shortname, "Coldfire-TLV320A");
	sprintf(card->longname, "Freescale Coldfire with TLV320A");

	err = snd_card_register(card);
	if (err == 0) {
		pr_debug(KERN_INFO "Coldfire audio support initialized\n");
		platform_set_drvdata(dev, card);
	}

	strcpy(chip->card->mixername, "TLV320A Volume");
	err = snd_ctl_add(chip->card, snd_ctl_new1(&tlv320_mixer_out, chip));
	if (err)
		goto error;
	err = snd_ctl_add(chip->card, snd_ctl_new1(&tlv320_mixer_linein, chip));
	if (err)
		goto error;
	err = snd_ctl_add(chip->card,
			  snd_ctl_new1(&tlv320_mixer_capture_source,
			  chip));
	if (err)
		goto error;
	err = snd_ctl_add(chip->card,
			  snd_ctl_new1(&tlv320_mixer_mic_boost,
			  chip));
	if (err)
		goto error;
	g_mastervol = TLV320A_VOL_INIT;
	g_lineinvol = TLV320A_LINEIN_INIT;
	g_analogpath = TLV320A_ANALOGPATH_INIT;
	ssi_audio_codec_reset();
	return 0;
error:
	kfree(card->private_data);
	snd_card_free(card);
	platform_set_drvdata(dev, NULL);
	return err;
}

static int coldfire_alsa_audio_remove(struct platform_device *dev)
{
	struct snd_card *card;

	card = platform_get_drvdata(dev);
	kfree(card->private_data);
	snd_card_free(card);
	platform_set_drvdata(dev, NULL);

	return 0;
}

static struct platform_driver coldfire_alsa_audio_driver = {
	.probe = coldfire_alsa_audio_probe,
	.remove = coldfire_alsa_audio_remove,
	.driver = {
		.name = SOUND_CARD_NAME,
		},
};

static int __init coldfire_alsa_audio_init(void)
{
	int err;
	err = platform_driver_register(&coldfire_alsa_audio_driver);
	if (err < 0)
		return err;

	device = platform_device_register_simple(SOUND_CARD_NAME, -1, NULL, 0);
	if (!IS_ERR(device)) {
		if (platform_get_drvdata(device))
			return 0;
		platform_device_unregister(device);
		platform_driver_unregister(&coldfire_alsa_audio_driver);
		err = -ENODEV;
	} else
		err = PTR_ERR(device);

	platform_driver_unregister(&coldfire_alsa_audio_driver);
	return err;
}

static void __exit coldfire_alsa_audio_exit(void)
{
	platform_device_unregister(device);
	platform_driver_unregister(&coldfire_alsa_audio_driver);
}

module_init(coldfire_alsa_audio_init);
module_exit(coldfire_alsa_audio_exit);

MODULE_DESCRIPTION("Coldfire driver for ALSA");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{TLV320A}}");

module_param(id, charp, 0444);
MODULE_PARM_DESC(id, "ID string for Coldfire + TLV320A soundcard.");
