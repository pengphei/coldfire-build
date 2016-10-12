/****************************************************************************/

/*
 *	fec.h  --  Fast Ethernet Controller for Motorola ColdFire SoC
 *		   processors.
 *
 *      Copyright 2008-2010 Freescale Semiconductor,Inc.All Rights Reserved.
 *	(C) Copyright 2000-2005, Greg Ungerer (gerg@snapgear.com)
 *	(C) Copyright 2000-2001, Lineo (www.lineo.com)
 */

/****************************************************************************/
#ifndef FEC_H
#define	FEC_H
/****************************************************************************/
/* The FEC stores dest/src/type, data, and checksum for receive packets.
 */
#define PKT_MAXBUF_SIZE         1518
#define PKT_MINBUF_SIZE         64
#define PKT_MAXBLR_SIZE         1520

/*
 * The 5270/5271/5280/5282/532x RX control register also contains maximum frame
 * size bits. Other FEC hardware does not, so we need to take that into
 * account when setting it.
 */
#if defined(CONFIG_M523x) || defined(CONFIG_M527x) || defined(CONFIG_M528x) || \
	defined(CONFIG_M520x) || defined(CONFIG_M532x) || \
	defined(CONFIG_M537x) || defined(CONFIG_M5301x) || \
	defined(CONFIG_M5445X) || defined(CONFIG_M5441X)
#define OPT_FRAME_SIZE  (PKT_MAXBUF_SIZE << 16)
#else
#define OPT_FRAME_SIZE  0
#endif
/*
 * Some hardware gets it MAC address out of local flash memory.
 * if this is non-zero then assume it is the address to get MAC from.
 */
#if defined(CONFIG_NETtel)
#define FEC_FLASHMAC    0xf0006006
#elif defined(CONFIG_GILBARCONAP) || defined(CONFIG_SCALES)
#define FEC_FLASHMAC    0xf0006000
#elif defined(CONFIG_CANCam)
#define FEC_FLASHMAC    0xf0020000
#elif defined(CONFIG_M5272C3)
#define FEC_FLASHMAC    (0xffe04000 + 4)
#elif defined(CONFIG_MOD5272)
#define FEC_FLASHMAC    0xffc0406b
#else
#define FEC_FLASHMAC    0
#endif

/* The number of Tx and Rx buffers.  These are allocated from the page
 * pool.  The code may assume these are power of two, so it it best
 * to keep them that size.
 * We don't need to allocate pages for the transmitter.  We just use
 * the skbuffer directly.
 */
#ifdef CONFIG_FEC_DMA_USE_SRAM
#define FEC_ENET_RX_PAGES       6
#else
#define FEC_ENET_RX_PAGES       8
#endif

#define FEC_ENET_RX_FRSIZE      2048
#define FEC_ENET_RX_FRPPG       (PAGE_SIZE / FEC_ENET_RX_FRSIZE)
#define RX_RING_SIZE            (FEC_ENET_RX_FRPPG * FEC_ENET_RX_PAGES)
#define FEC_ENET_TX_FRSIZE      2048
#define FEC_ENET_TX_FRPPG       (PAGE_SIZE / FEC_ENET_TX_FRSIZE)

#ifdef CONFIG_FEC_DMA_USE_SRAM
#define TX_RING_SIZE            8      /* Must be power of two */
#define TX_RING_MOD_MASK        7      /*   for this to work */
#else
#define TX_RING_SIZE            16      /* Must be power of two */
#define TX_RING_MOD_MASK        15      /*   for this to work */
#endif

#if (((RX_RING_SIZE + TX_RING_SIZE) * 8) > PAGE_SIZE)
#error "FEC: descriptor ring size constants too large"
#endif

#if defined(CONFIG_M523x) || defined(CONFIG_M527x) || defined(CONFIG_M528x) || \
    defined(CONFIG_M520x) || defined(CONFIG_M532x) || defined(CONFIG_M537x) || \
    defined(CONFIG_M5301x) || defined(CONFIG_M5445X) || defined(CONFIG_M5441X)
/* Define the FEC 1588 registers offset */
#if defined(CONFIG_FEC_1588)
#define FEC_ATIME_CTRL          0x400
#define FEC_ATIME               0x404
#define FEC_ATIME_EVT_OFFSET    0x408
#define FEC_ATIME_EVT_PERIOD    0x40c
#define FEC_ATIME_CORR          0x410
#define FEC_ATIME_INC           0x414
#define FEC_TS_TIMESTAMP        0x418
#endif
/*
 *	Just figures, Motorola would have to change the offsets for
 *	registers in the same peripheral device on different models
 *	of the ColdFire!
 */
typedef struct fec {
	unsigned long	fec_reserved0;
	unsigned long	fec_ievent;		/* Interrupt event reg */
	unsigned long	fec_imask;		/* Interrupt mask reg */
	unsigned long	fec_reserved1;
	unsigned long	fec_r_des_active;	/* Receive descriptor reg */
	unsigned long	fec_x_des_active;	/* Transmit descriptor reg */
	unsigned long	fec_reserved2[3];
	unsigned long	fec_ecntrl;		/* Ethernet control reg */
	unsigned long	fec_reserved3[6];
	unsigned long	fec_mii_data;		/* MII manage frame reg */
	unsigned long	fec_mii_speed;		/* MII speed control reg */
	unsigned long	fec_reserved4[7];
	unsigned long	fec_mib_ctrlstat;	/* MIB control/status reg */
	unsigned long	fec_reserved5[7];
	unsigned long	fec_r_cntrl;		/* Receive control reg */
	unsigned long	fec_reserved6[15];
	unsigned long	fec_x_cntrl;		/* Transmit Control reg */
	unsigned long	fec_reserved7[7];
	unsigned long	fec_addr_low;		/* Low 32bits MAC address */
	unsigned long	fec_addr_high;		/* High 16bits MAC address */
	unsigned long	fec_opd;		/* Opcode + Pause duration */
	unsigned long	fec_reserved8[10];
	unsigned long	fec_hash_table_high;	/* High 32bits hash table */
	unsigned long	fec_hash_table_low;	/* Low 32bits hash table */
	unsigned long	fec_grp_hash_table_high;/* High 32bits hash table */
	unsigned long	fec_grp_hash_table_low;	/* Low 32bits hash table */
	unsigned long	fec_reserved9[7];
	unsigned long	fec_x_wmrk;		/* FIFO transmit water mark */
	unsigned long	fec_reserved10;
	unsigned long	fec_r_bound;		/* FIFO receive bound reg */
	unsigned long	fec_r_fstart;		/* FIFO receive start reg */
	unsigned long	fec_reserved11[11];
	unsigned long	fec_r_des_start;	/* Receive descriptor ring */
	unsigned long	fec_x_des_start;	/* Transmit descriptor ring */
	unsigned long	fec_r_buff_size;	/* Maximum receive buff size */
} fec_t;

/* Define the FEC 1588 registers offset */
#define FEC_ATIME_CTRL		0x400
#define FEC_ATIME		0x404
#define FEC_ATIME_EVT_OFFSET    0x408
#define FEC_ATIME_EVT_PERIOD	0x40c
#define FEC_ATIME_CORR		0x410
#define FEC_ATIME_INC		0x414
#define FEC_TS_TIMESTAMP	0x418

#else

/*
 *	Define device register set address map.
 */
typedef struct fec {
	unsigned long	fec_ecntrl;		/* Ethernet control reg */
	unsigned long	fec_ievent;		/* Interrupt even reg */
	unsigned long	fec_imask;		/* Interrupt mask reg */
	unsigned long	fec_ivec;		/* Interrupt vec status reg */
	unsigned long	fec_r_des_active;	/* Receive descriptor reg */
	unsigned long	fec_x_des_active;	/* Transmit descriptor reg */
	unsigned long	fec_reserved1[10];
	unsigned long	fec_mii_data;		/* MII manage frame reg */
	unsigned long	fec_mii_speed;		/* MII speed control reg */
	unsigned long	fec_reserved2[17];
	unsigned long	fec_r_bound;		/* FIFO receive bound reg */
	unsigned long	fec_r_fstart;		/* FIFO receive start reg */
	unsigned long	fec_reserved3[4];
	unsigned long	fec_x_wmrk;		/* FIFO transmit water mark */
	unsigned long	fec_reserved4;
	unsigned long	fec_x_fstart;		/* FIFO transmit start reg */
	unsigned long	fec_reserved5[21];
	unsigned long	fec_r_cntrl;		/* Receive control reg */
	unsigned long	fec_max_frm_len;	/* Maximum frame length reg */
	unsigned long	fec_reserved6[14];
	unsigned long	fec_x_cntrl;		/* Transmit Control reg */
	unsigned long	fec_reserved7[158];
	unsigned long	fec_addr_low;		/* Low 32bits MAC address */
	unsigned long	fec_addr_high;		/* High 16bits MAC address */
	unsigned long	fec_grp_hash_table_high;/* High 32bits hash table */
	unsigned long	fec_grp_hash_table_low;	/* Low 32bits hash table */
	unsigned long	fec_r_des_start;	/* Receive descriptor ring */
	unsigned long	fec_x_des_start;	/* Transmit descriptor ring */
	unsigned long	fec_r_buff_size;	/* Maximum receive buff size */
	unsigned long	reserved8[9];
	unsigned long	fec_fifo_ram[112];	/* FIFO RAM buffer */
} fec_t;

#endif /* CONFIG_M5272 */

#if defined(CONFIG_FEC_1588)
#define FEC_ENHANCED_MODE       1
#endif
/*
 *	Define the buffer descriptor structure.
 */
typedef struct bufdesc {
	unsigned short	cbd_sc;			/* Control and status info */
	unsigned short	cbd_datlen;		/* Data length */
	unsigned long	cbd_bufaddr;		/* Buffer address */
#ifdef FEC_ENHANCED_MODE
	unsigned long cbd_esc;
	unsigned long cbd_prot;
	unsigned long cbd_bdu;
	unsigned long ts;
	unsigned short res0[4];
#endif

} cbd_t;

/* Forward declarations of some structures to support different PHYs
 */
typedef struct {
	uint mii_data;
	void (*funct)(uint mii_reg, struct net_device *dev);
} phy_cmd_t;

typedef struct {
	uint id;
	char *name;

	const phy_cmd_t *config;
	const phy_cmd_t *startup;
	const phy_cmd_t *ack_int;
	const phy_cmd_t *shutdown;
} phy_info_t;

/*
 *	The following definitions courtesy of commproc.h, which where
 *	Copyright (c) 1997 Dan Malek (dmalek@jlc.net).
 */
#define BD_SC_EMPTY     ((ushort)0x8000)        /* Recieve is empty */
#define BD_SC_READY     ((ushort)0x8000)        /* Transmit is ready */
#define BD_SC_WRAP      ((ushort)0x2000)        /* Last buffer descriptor */
#define BD_SC_INTRPT    ((ushort)0x1000)        /* Interrupt on change */
#define BD_SC_CM        ((ushort)0x0200)        /* Continous mode */
#define BD_SC_ID        ((ushort)0x0100)        /* Rec'd too many idles */
#define BD_SC_P         ((ushort)0x0100)        /* xmt preamble */
#define BD_SC_BR        ((ushort)0x0020)        /* Break received */
#define BD_SC_FR        ((ushort)0x0010)        /* Framing error */
#define BD_SC_PR        ((ushort)0x0008)        /* Parity error */
#define BD_SC_OV        ((ushort)0x0002)        /* Overrun */
#define BD_SC_CD        ((ushort)0x0001)        /* ?? */

/* Buffer descriptor control/status used by Ethernet receive.
*/
#define BD_ENET_RX_EMPTY        ((ushort)0x8000)
#define BD_ENET_RX_WRAP         ((ushort)0x2000)
#define BD_ENET_RX_INTR         ((ushort)0x1000)
#define BD_ENET_RX_LAST         ((ushort)0x0800)
#define BD_ENET_RX_FIRST        ((ushort)0x0400)
#define BD_ENET_RX_MISS         ((ushort)0x0100)
#define BD_ENET_RX_LG           ((ushort)0x0020)
#define BD_ENET_RX_NO           ((ushort)0x0010)
#define BD_ENET_RX_SH           ((ushort)0x0008)
#define BD_ENET_RX_CR           ((ushort)0x0004)
#define BD_ENET_RX_OV           ((ushort)0x0002)
#define BD_ENET_RX_CL           ((ushort)0x0001)
#define BD_ENET_RX_STATS        ((ushort)0x013f)        /* All status bits */

#define BD_ENET_RX_INT          0x00800000

#define BD_ENET_RX_INT		0x00800000

/* Buffer descriptor control/status used by Ethernet transmit.
*/
#define BD_ENET_TX_READY        ((ushort)0x8000)
#define BD_ENET_TX_PAD          ((ushort)0x4000)
#define BD_ENET_TX_WRAP         ((ushort)0x2000)
#define BD_ENET_TX_INTR         ((ushort)0x1000)
#define BD_ENET_TX_LAST         ((ushort)0x0800)
#define BD_ENET_TX_TC           ((ushort)0x0400)
#define BD_ENET_TX_DEF          ((ushort)0x0200)
#define BD_ENET_TX_HB           ((ushort)0x0100)
#define BD_ENET_TX_LC           ((ushort)0x0080)
#define BD_ENET_TX_RL           ((ushort)0x0040)
#define BD_ENET_TX_RCMASK       ((ushort)0x003c)
#define BD_ENET_TX_UN           ((ushort)0x0002)
#define BD_ENET_TX_CSL          ((ushort)0x0001)
#define BD_ENET_TX_STATS        ((ushort)0x03ff)        /* All status bits */

#define BD_ENET_TX_INT          0x40000000
/****************************************************************************/
#endif /* FEC_H */
