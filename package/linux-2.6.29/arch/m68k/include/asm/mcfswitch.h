/****************************************************************************/

/*
 *	mcfswitch --  L2 SWITCH  Controller for Motorola ColdFire SoC
 *		   processors.
 *
 *  Copyright (C) 2010 Freescale Semiconductor, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/****************************************************************************/
#ifndef SWITCH_H
#define	SWITCH_H
/****************************************************************************/
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <asm/pgtable.h>

/*
 * Some hardware gets it MAC address out of local flash memory.
 * if this is non-zero then assume it is the address to get MAC from.
 */
#define FEC_FLASHMAC    0

#ifdef CONFIG_SWITCH_DMA_USE_SRAM
#define TX_RING_SIZE            8      /* Must be power of two */
#define TX_RING_MOD_MASK        7      /*   for this to work */
#else
#define TX_RING_SIZE            16      /* Must be power of two */
#define TX_RING_MOD_MASK        15      /*   for this to work */
#endif

typedef struct l2switch_port_status {
	/*outgoing frames discarded due to transmit queue congestion*/
	unsigned long MCF_ESW_POQC;
	/*incoming frames discarded due to VLAN domain mismatch*/
	unsigned long MCF_ESW_PMVID;
	/*incoming frames discarded due to untagged discard*/
	unsigned long MCF_ESW_PMVTAG;
	/*incoming frames discarded due port is in blocking state*/
	unsigned long MCF_ESW_PBL;
} esw_port_stats;

typedef struct l2switch {
	unsigned long ESW_REVISION;
	unsigned long ESW_SCRATCH;
	unsigned long ESW_PER;
	unsigned long reserved0[1];
	unsigned long ESW_VLANV;
	unsigned long ESW_DBCR;
	unsigned long ESW_DMCR;
	unsigned long ESW_BKLR;
	unsigned long ESW_BMPC;
	unsigned long ESW_MODE;
	unsigned long ESW_VIMSEL;
	unsigned long ESW_VOMSEL;
	unsigned long ESW_VIMEN;
	unsigned long ESW_VID;/*0x34*/
	/*from 0x38 0x3C*/
	unsigned long esw_reserved0[2];
	unsigned long ESW_MCR;/*0x40*/
	unsigned long ESW_EGMAP;
	unsigned long ESW_INGMAP;
	unsigned long ESW_INGSAL;
	unsigned long ESW_INGSAH;
	unsigned long ESW_INGDAL;
	unsigned long ESW_INGDAH;
	unsigned long ESW_ENGSAL;
	unsigned long ESW_ENGSAH;
	unsigned long ESW_ENGDAL;
	unsigned long ESW_ENGDAH;
	unsigned long ESW_MCVAL;/*0x6C*/
	/*from 0x70--0x7C*/
	unsigned long esw_reserved1[4];
	unsigned long ESW_MMSR;/*0x80*/
	unsigned long ESW_LMT;
	unsigned long ESW_LFC;
	unsigned long ESW_PCSR;
	unsigned long ESW_IOSR;
	unsigned long ESW_QWT;/*0x94*/
	unsigned long esw_reserved2[1];/*0x98*/
	unsigned long ESW_P0BCT;/*0x9C*/
	/*from 0xA0-0xB8*/
	unsigned long esw_reserved3[7];
	unsigned long ESW_P0FFEN;/*0xBC*/
	/*MCF_ESW_PSNP(x)   0xFC0DC0C0+((x-1)*0x004)))  0xC0-0xDC*/
	/*#define MCF_ESW_PSNP(x)  \
	   (*(volatile unsigned long*)(0xFC0DC0C0+((x-1)*0x004)))*/
	unsigned long ESW_PSNP[8];
	/*MCF_ESW_IPSNP(x)  0xFC0DC0E0+((x-1)*0x004)    0xE0-0xFC*/
	/*#define MCF_ESW_IPSNP(x)  \
	   (*(volatile unsigned long*)(0xFC0DC0E0+((x-1)*0x004)))*/
	unsigned long ESW_IPSNP[8];
	/*port0-port2 VLAN Priority resolution map  0xFC0D_C100-C108*/
	/*#define MCF_ESW_PVRES(x)  \
	   (*(volatile unsigned long*)(0xFC0DC100+((x)*0x004)))*/
	unsigned long ESW_PVRES[3];
	/*from 0x10C-0x13C*/
	unsigned long esw_reserved4[13];
	unsigned long ESW_IPRES;/*0x140*/
	/*from 0x144-0x17C*/
	unsigned long esw_reserved5[15];

	/*port0-port2 Priority Configuration  0xFC0D_C180-C188*/
	/*#define MCF_ESW_PRES(x)   \
	    (*(volatile unsigned long*)(0xFC0DC180+((x)*0x004)))*/
	unsigned long ESW_PRES[3];
	/*from 0x18C-0x1FC*/
	unsigned long esw_reserved6[29];

	/*port0-port2 VLAN ID 0xFC0D_C200-C208*/
	/*#define MCF_ESW_PID(x)   \
	    (*(volatile unsigned long*)(0xFC0DC200+((x)*0x004)))*/
	unsigned long ESW_PID[3];
	/*from 0x20C-0x27C*/
	unsigned long esw_reserved7[29];

	/*port0-port2 VLAN domain resolution entry 0xFC0D_C280-C2FC*/
	/*#define MCF_ESW_VRES(x)  \
	    (*(volatile unsigned long*)(0xFC0DC280+((x)*0x004)))*/
	unsigned long ESW_VRES[32];

	unsigned long ESW_DISCN;/*0x300*/
	unsigned long ESW_DISCB;
	unsigned long ESW_NDISCN;
	unsigned long ESW_NDISCB;/*0xFC0DC30C*/
	/*per port statistics 0xFC0DC310_C33C*/
	/*#define MCF_ESW_POQC(x)   \
	    (*(volatile unsigned long*)(0xFC0DC310+((x)*0x010)))
	#define MCF_ESW_PMVID(x)  \
	    (*(volatile unsigned long*)(0xFC0DC314+((x)*0x010)))
	#define MCF_ESW_PMVTAG(x) \
	    (*(volatile unsigned long*)(0xFC0DC318+((x)*0x010)))
	#define MCF_ESW_PBL(x)    \
	    (*(volatile unsigned long*)(0xFC0DC31C+((x)*0x010)))
	*/
	esw_port_stats portstats[3];
	/*from 0x340-0x400*/
	unsigned long esw_reserved8[48];

	/*0xFC0DC400---0xFC0DC418*/
	/*unsigned long MCF_ESW_ISR;*/
	unsigned long   switch_ievent;             /* Interrupt event reg */
	/*unsigned long MCF_ESW_IMR;*/
	unsigned long   switch_imask;              /* Interrupt mask reg */
	/*unsigned long MCF_ESW_RDSR;*/
	unsigned long   fec_r_des_start;        /* Receive descriptor ring */
	/*unsigned long MCF_ESW_TDSR;*/
	unsigned long   fec_x_des_start;        /* Transmit descriptor ring */
	/*unsigned long MCF_ESW_MRBR;*/
	unsigned long   fec_r_buff_size;        /* Maximum receive buff size */
	/*unsigned long MCF_ESW_RDAR;*/
	unsigned long   fec_r_des_active;       /* Receive descriptor reg */
	/*unsigned long MCF_ESW_TDAR;*/
	unsigned long   fec_x_des_active;       /* Transmit descriptor reg */
	/*from 0x420-0x4FC*/
	unsigned long esw_reserved9[56];

	/*0xFC0DC500---0xFC0DC508*/
	unsigned long ESW_LREC0;
	unsigned long ESW_LREC1;
	unsigned long ESW_LSR;
} switch_t;

typedef struct _64bTableEntry {
	unsigned int lo;  /* lower 32 bits */
	unsigned int hi;  /* upper 32 bits */
} AddrTable64bEntry;

typedef struct l2switchaddrtable {
	AddrTable64bEntry  eswTable64bEntry[2048];
} eswAddrTable_t;

/*unsigned long MCF_ESW_LOOKUP_MEM;*/
#define MCF_ESW_REVISION   (*(volatile unsigned long*)(0xFC0DC000))
#define MCF_ESW_PER        (*(volatile unsigned long*)(0xFC0DC008))
#define MCF_ESW_VLANV      (*(volatile unsigned long*)(0xFC0DC010))
#define MCF_ESW_DBCR       (*(volatile unsigned long*)(0xFC0DC014))
#define MCF_ESW_DMCR       (*(volatile unsigned long*)(0xFC0DC018))
#define MCF_ESW_BKLR       (*(volatile unsigned long*)(0xFC0DC01C))
#define MCF_ESW_BMPC       (*(volatile unsigned long*)(0xFC0DC020))
#define MCF_ESW_MODE       (*(volatile unsigned long*)(0xFC0DC024))

#define MCF_ESW_ISR        (*(volatile unsigned long*)(0xFC0DC400))
#define MCF_ESW_IMR        (*(volatile unsigned long*)(0xFC0DC404))
#define MCF_ESW_TDAR       (*(volatile unsigned long*)(0xFC0DC418))
#define MCF_ESW_LOOKUP_MEM (*(volatile unsigned long*)(0xFC0E0000))

#define MCF_PPMCR0   (*(volatile unsigned short*)(0xFC04002D))
#define MCF_PPMHR0   (*(volatile unsigned long*)(0xFC040030))

#define MCF_FEC_MSCR0      (*(volatile unsigned long*)(0xFC0D4044))
#define MCF_FEC_MSCR1      (*(volatile unsigned long*)(0xFC0D8044))
#define MCF_FEC_RCR0       (*(volatile unsigned long*)(0xFC0D4084))
#define MCF_FEC_RCR1       (*(volatile unsigned long*)(0xFC0D8084))
#define MCF_FEC_TCR0       (*(volatile unsigned long*)(0xFC0D40C4))
#define MCF_FEC_TCR1       (*(volatile unsigned long*)(0xFC0D80C4))
#define MCF_FEC_ECR0       (*(volatile unsigned long*)(0xFC0D4024))
#define MCF_FEC_ECR1       (*(volatile unsigned long*)(0xFC0D8024))


#define MCF_FEC_RCR_PROM                     (0x00000008)
#define MCF_FEC_RCR_RMII_MODE                (0x00000100)
#define MCF_FEC_RCR_MAX_FL(x)                (((x)&0x00003FFF)<<16)
#define MCF_FEC_RCR_CRC_FWD                  (0x00004000)

#define MCF_FEC_TCR_FDEN                     (0x00000004)

#define MCF_FEC_ECR_ETHER_EN                 (0x00000002)
#define MCF_FEC_ECR_ENA_1588                 (0x00000010)

/*
 * Info received from Hardware Learning FIFO,
 * holding MAC address and corresponding Hash Value and
 * port number where the frame was received (disassembled).
 */
typedef struct _eswPortInfo {
	/* MAC lower 32 bits (first byte is 7:0). */
	unsigned int   maclo;
	/* MAC upper 16 bits (47:32). */
	unsigned int   machi;
	/* the hash value for this MAC address. */
	unsigned int   hash;
	/* the port number this MAC address is associated with. */
	unsigned int   port;
} eswPortInfo;

/*
 * Hardware Look up Address Table 64-bit element.
 */
typedef volatile struct _64bitTableEntry {
	unsigned int lo;  /* lower 32 bits */
	unsigned int hi;  /* upper 32 bits */
} eswTable64bitEntry;

/*
 * Disassembled element stored in Address Table.
 */
typedef struct _addrtableDynamicEntry {
	/* MAC lower 32 bits (first byte is 7:0). */
	unsigned int maclo;
	/* MAC upper 16 bits (47:32). */
	unsigned int machi;
	/* timestamp of this entry */
	unsigned int timestamp;
	/* the port number this MAC address is associated with */
	unsigned int port;
} eswAddrTableDynamicEntry;

typedef struct _addrtableStaticEntry {
	/* MAC lower 32 bits (first byte is 7:0). */
	unsigned int maclo;
	/* MAC upper 16 bits (47:32). */
	unsigned int machi;
	/* priority of this entry */
	unsigned int priority;
	/* the port bitmask this MAC address is associated with */
	unsigned int portbitmask;
} eswAddrTableStaticEntry;
/*
 *	Define the buffer descriptor structure.
 */
typedef struct bufdesc {
	unsigned short	cbd_sc;			/* Control and status info */
	unsigned short	cbd_datlen;		/* Data length */
	unsigned long	cbd_bufaddr;		/* Buffer address */
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

/* The FEC buffer descriptors track the ring buffers.  The rx_bd_base and
 * tx_bd_base always point to the base of the buffer descriptors.  The
 * cur_rx and cur_tx point to the currently available buffer.
 * The dirty_tx tracks the current buffer that is being sent by the
 * controller.  The cur_tx and dirty_tx are equal under both completely
 * empty and completely full conditions.  The empty/ready indicator in
 * the buffer descriptor determines the actual condition.
 */
struct switch_enet_private {
	/* Hardware registers of the FEC device */
	volatile switch_t  *hwp;
	volatile eswAddrTable_t  *hwentry;

	struct net_device *netdev;
	struct platform_device *pdev;
	/* The saved address of a sent-in-place packet/buffer, for skfree(). */
	unsigned char *tx_bounce[TX_RING_SIZE];
	struct  sk_buff *tx_skbuff[TX_RING_SIZE];
	ushort  skb_cur;
	ushort  skb_dirty;

	/* CPM dual port RAM relative addresses.
	 */
	cbd_t   *rx_bd_base;            /* Address of Rx and Tx buffers. */
	cbd_t   *tx_bd_base;
	cbd_t   *cur_rx, *cur_tx;               /* The next free ring entry */
	cbd_t   *dirty_tx;      /* The ring entries to be free()ed. */
	uint    tx_full;
	/* hold while accessing the HW like ringbuffer for tx/rx but not MAC */
	spinlock_t hw_lock;

	/* hold while accessing the mii_list_t() elements */
	spinlock_t mii_lock;
	struct mii_bus *mdio_bus;
	struct phy_device *phydev;

	uint    phy_id;
	uint    phy_id_done;
	uint    phy_status;
	uint    phy_speed;
	phy_info_t const        *phy;
	struct work_struct phy_task;
	volatile switch_t  *phy_hwp;

	uint    sequence_done;
	uint    mii_phy_task_queued;

	uint    phy_addr;

	int     index;
	int     opened;
	int     link;
	int     old_link;
	int     full_duplex;
	int     duplex;
	int     speed;
	int     msg_enable;

	/* --------------Statistics--------------------------- */
	/* when a new element deleted a element with in
	 * a block due to lack of space */
	int atBlockOverflows;
	/* Peak number of valid entries in the address table */
	int atMaxEntries;
	/* current number of valid entries in the address table */
	int atCurrEntries;
	/* maximum entries within a block found
	 * (updated within ageing)*/
	int atMaxEntriesPerBlock;

	/* -------------------ageing function------------------ */
	/* maximum age allowed for an entry */
	int ageMax;
	/* last LUT entry to block that was
	 * inspected by the Ageing task*/
	int ageLutIdx;
	/* last element within block inspected by the Ageing task */
	int ageBlockElemIdx;
	/* complete table has been processed by ageing process */
	int ageCompleted;
	/* delay setting */
	int ageDelay;
	/* current delay Counter */
	int  ageDelayCnt;

	/* ----------------timer related---------------------------- */
	/* current time (for timestamping) */
	int currTime;
	/* flag set by timer when currTime changed
	 * and cleared by serving function*/
	int timeChanged;

	/**/
	/* Timer for Aging */
	struct timer_list       timer_aging;
};

struct switch_platform_private {
	struct platform_device  *pdev;

	unsigned long           quirks;
	int                     num_slots;      /* Slots on controller */
	struct switch_enet_private *fep_host[0];      /* Pointers to hosts */
};


#endif
