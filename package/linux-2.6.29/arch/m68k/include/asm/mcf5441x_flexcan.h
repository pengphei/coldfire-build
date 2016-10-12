/*
 * Copyright (C) 2010 Freescale Semiconductor, Inc. All rights reserved.
 *
 * Author: Lanttor Guo <b18203@freescale.com>
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef MCF5441X_FLEXCAN_H
#define MCF5441X_FLEXCAN_H
/*
 *      FlexCAN Module Configuration Register
 */
#define CANMCR_MDIS		(0x80000000)
#define CANMCR_FRZ		(0x40000000)
#define CANMCR_FEN		(0x20000000)
#define CANMCR_HALT		(0x10000000)
#define CANMCR_SOFTRST		(0x02000000)
#define CANMCR_NOTRDY		(0x08000000)
#define CANMCR_FRZACK		(0x01000000)
#define CANMCR_SUPV		(0x00800000)
#define CANMCR_WARN_EN		(0x00200000)
#define CANMCR_LPMACK		(0x00100000)
#define CANMCR_DOZE		(0x00040000)
#define CANMCR_SRXDIS		(0x00020000)
#define CANMCR_BCC		(0x00010000)
#define CANMCR_LPRIO_EN		(0x00002000)
#define CANMCR_AEN		(0x00001000)
#define CANMCR_IDAM_VAL(x)	((x & 0x3) << 8)
#define CANMCR_MAXMB		(0x0F)
/*
 *      FlexCAN Control Register
 */
#define CANCTRL_PRESDIV(x)      (((x)&0xFF)<<24)
#define CANCTRL_RJW(x)          (((x)&0x03)<<22)
#define CANCTRL_PSEG1(x)        (((x)&0x07)<<19)
#define CANCTRL_PSEG2(x)        (((x)&0x07)<<16)
#define CANCTRL_BOFFMSK         (0x00008000)
#define CANCTRL_ERRMSK	        (0x00004000)
#define CANCTRL_CLKSRC		(0x00002000)
#define CANCTRL_LPB	        (0x00001000)
#define CANCTRL_TWRN_MSK	(0x00000800)
#define CANCTRL_RWRN_MSK	(0x00000400)
#define CANCTRL_SAMP(x)	        (((x)&0x01)<<7)
#define CANCTRL_BOFFREC         (0x00000040)
#define CANCTRL_TSYNC           (0x00000020)
#define CANCTRL_LBUF            (0x00000010)
#define CANCTRL_LOM             (0x00000008)
#define CANCTRL_PROPSEG(x)      ((x)&0x07)

/*
 *      FlexCAN Error Counter Register
 */
#define ERRCNT_RXECTR(x)        (((x)&0xFF)<<8)
#define ERRCNT_TXECTR(x)        ((x)&0xFF)

/*
 *      FlexCAN Error and Status Register
 */
#define ERRSTAT_TWRN_INT	 (0x00020000)
#define ERRSTAT_RWRN_INT	 (0x00010000)
#define ERRSTAT_BITERR(x)       (((x)&0x03)<<14)
#define ERRSTAT_ACKERR           (0x00002000)
#define ERRSTAT_CRCERR           (0x00001000)
#define ERRSTAT_FRMERR           (0x00000800)
#define ERRSTAT_STFERR           (0x00000400)
#define ERRSTAT_TXWRN            (0x00000200)
#define ERRSTAT_RXWRN            (0x00000100)
#define ERRSTAT_IDLE             (0x00000080)
#define ERRSTAT_TXRX             (0x00000040)
#define ERRSTAT_FLTCONF(x)       (((x)&0x03)<<4)
#define ERRSTAT_BOFFINT          (0x00000004)
#define ERRSTAT_ERRINT           (0x00000002)

/*
 *      Interrupt Mask Register
 */
#define IMASK_BUF15M		(0x8000)
#define IMASK_BUF14M		(0x4000)
#define IMASK_BUF13M		(0x2000)
#define IMASK_BUF12M		(0x1000)
#define IMASK_BUF11M		(0x0800)
#define IMASK_BUF10M		(0x0400)
#define IMASK_BUF9M		(0x0200)
#define IMASK_BUF8M		(0x0100)
#define IMASK_BUF7M		(0x0080)
#define IMASK_BUF6M		(0x0040)
#define IMASK_BUF5M		(0x0020)
#define IMASK_BUF4M		(0x0010)
#define IMASK_BUF3M		(0x0008)
#define IMASK_BUF2M		(0x0004)
#define IMASK_BUF1M		(0x0002)
#define IMASK_BUF0M		(0x0001)
#define IMASK_BUFnM(x)		(0x1<<(x))
#define IMASK_BUFF_ENABLE_ALL	(0xFFFF)
#define IMASK_BUFF_DISABLE_ALL	(0x0000)

/*
 *      Interrupt Flag Register
 */
#define IFLAG_BUF15M		(0x8000)
#define IFLAG_BUF14M		(0x4000)
#define IFLAG_BUF13M		(0x2000)
#define IFLAG_BUF12M		(0x1000)
#define IFLAG_BUF11M		(0x0800)
#define IFLAG_BUF10M		(0x0400)
#define IFLAG_BUF9M		(0x0200)
#define IFLAG_BUF8M		(0x0100)
#define IFLAG_BUF7M		(0x0080)
#define IFLAG_BUF6M		(0x0040)
#define IFLAG_BUF5M		(0x0020)
#define IFLAG_BUF4M		(0x0010)
#define IFLAG_BUF3M		(0x0008)
#define IFLAG_BUF2M		(0x0004)
#define IFLAG_BUF1M		(0x0002)
#define IFLAG_BUF0M		(0x0001)
#define IFLAG_BUFF_SET_ALL	(0xFFFF)
#define IFLAG_BUFF_CLEAR_ALL	(0x0000)
#define IFLAG_BUFnM(x)		(0x1<<(x))

/*
 *      Message Buffers
 */
#define MB_CNT_CODE(x)		(((x)&0x0F)<<24)
#define MB_CNT_SRR		(0x00400000)
#define MB_CNT_IDE		(0x00200000)
#define MB_CNT_RTR		(0x00100000)
#define MB_CNT_TIMESTAMP(x)	((x)&0xFFFF)
#define MB_PRIO(x)		(((x)&0x7)<<29)
#define MB_ID_STD		(0x07FF)
#define MB_ID_EXT		(0x1FFFFFFF)
#define MB_CODE_MASK		(0xF0FFFFFF)
#define CAN_MB			16
#define PDEV_MAX		2

#endif
