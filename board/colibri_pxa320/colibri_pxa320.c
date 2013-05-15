/*
 * Toradex Colibri PXA320 support file
 *
 * Copyright (C) 2009 Marek Vasut <marek.vasut@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/pxa-regs.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define	CPLD_REG_CS_CTRL			0x17800000
#define	CPLD_REG_CS_CTRL_EXT_nCS0_EN		(1 << 0)
#define	CPLD_REG_CS_CTRL_EXT_nCS1_EN		(1 << 1)
#define	CPLD_REG_CS_CTRL_EXT_nCS2_EN		(1 << 2)
#define	CPLD_REG_CS_CTRL_EXT_nCS0_EC_EN		(1 << 5)
#define	CPLD_REG_CS_CTRL_EXT_nCS1_EC_EN		(1 << 6)
#define	CPLD_REG_CS_CTRL_EXT_nCS2_EC_EN		(1 << 7)
#define	CPLD_REG_CS_CTRL_EXT_nCS0_DIS		(1 << 8)
#define	CPLD_REG_CS_CTRL_EXT_nCS1_DIS		(1 << 9)
#define	CPLD_REG_CS_CTRL_EXT_nCS2_DIS		(1 << 10)
#define	CPLD_REG_CS_CTRL_EXT_nCS0_EC_DIS	(1 << 11)
#define	CPLD_REG_CS_CTRL_EXT_nCS1_EC_DIS	(1 << 12)
#define	CPLD_REG_CS_CTRL_EXT_nCS2_EC_DIS	(1 << 13)

#define	CPLD_REG_CS_MEM				0x17800004
#define	CPLD_REG_CS_MEM_CF_EN			(1 << 0)
#define	CPLD_REG_CS_MEM_RDnWR_EN		(1 << 1)
#define	CPLD_REG_CS_MEM_nOE_EN			(1 << 2)
#define	CPLD_REG_CS_MEM_CF_DIS			(1 << 8)
#define	CPLD_REG_CS_MEM_RDnWR_DIS		(1 << 9)
#define	CPLD_REG_CS_MEM_nOE_DIS			(1 << 10)

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init (void)
{
	/* arch number for linux kernel */
	gd->bd->bi_arch_number = MACH_TYPE_COLIBRI320;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	/* Ethernet chip configuration */
	writel(0x00000801, GPIO3);
	writel(0x7ff8023c, MSC1);
	writel(0x0032c80b, CSADRCFG2);

	/* Chipselect 3 configuration */
	writel(0x00000801, GPIO4);
	writel(0x0032c809, CSADRCFG3);

	/* Configuration for Compact Flash */
	writel(0x0038080c, CSADRCFG_P);
	writel(0x00000002, CSMSADRCFG);

	/* Configuration for the CPLD */
#if 0	/* Disabled due to bug in CPLD FW v1.6 */
	writew (CPLD_REG_CS_CTRL_EXT_nCS0_EN |
		CPLD_REG_CS_CTRL_EXT_nCS1_EN |
		CPLD_REG_CS_CTRL_EXT_nCS2_EN,
		CPLD_REG_CS_CTRL);
#endif
	writew (CPLD_REG_CS_MEM_CF_EN |
		CPLD_REG_CS_MEM_RDnWR_EN |
		CPLD_REG_CS_MEM_nOE_EN,
		CPLD_REG_CS_MEM);

	/* CPLD programming interface */
	writel(0xd887, GPIO1);
	writel(0xd880, GPIO1_2);
	writel(0xd880, GPIO83);
	writel(0xd880, GPIO85);
	writel(0xd880, GPIO86);

	writel(0x2, GPCR0);
	writel(readl(GPDR0) | 0x2, GPDR0);

	writel(0x00680000, GPCR2);
	writel(readl(GPDR2) | 0x00480000, GPDR2);
	writel(readl(GPDR2) & ~0x00200000, GPDR2);

	/* MMC */
	writel(0x804, GPIO18);
	writel(0x804, GPIO19);
	writel(0x804, GPIO20);
	writel(0x804, GPIO21);
	writel(0x804, GPIO22);
	writel(0x804, GPIO23);

	return 0;
}

int board_late_init(void)
{
	setenv("stdout", "serial");
	setenv("stderr", "serial");
	return 0;
}

#ifdef	CONFIG_CMD_USB
int usb_board_init(void)
{
	writel((readl(UHCHR) | UHCHR_PCPL | UHCHR_PSPL) &
		~(UHCHR_SSEP0 | UHCHR_SSEP1 | UHCHR_SSEP2 | UHCHR_SSE),
		UHCHR);

	writel(readl(UHCHR) | UHCHR_FSBIR, UHCHR);

	while (UHCHR & UHCHR_FSBIR);

	writel(readl(UHCHR) & ~UHCHR_SSE, UHCHR);
	writel((UHCHIE_UPRIE | UHCHIE_RWIE), UHCHIE);

	writel(readl(UHCRHDA) & ~(0x200), UHCRHDA);
	writel(readl(UHCRHDA) | 0x100, UHCRHDA);

	/* Set port power control mask bits, only 3 ports. */
	writel(readl(UHCRHDB) | (0x7<<17), UHCRHDB);

	/* enable port 2 */
	writel(readl(UP2OCR) | UP2OCR_HXOE | UP2OCR_HXS |
		UP2OCR_DMPDE | UP2OCR_DPPDE, UP2OCR);

	return 0;
}

void usb_board_init_fail(void)
{
	return;
}

void usb_board_stop(void)
{
	writel(readl(UHCHR) | UHCHR_FHR, UHCHR);
	udelay(11);
	writel(readl(UHCHR) & ~UHCHR_FHR, UHCHR);

	writel(readl(UHCCOMS) | 1, UHCCOMS);
	udelay(10);

//	writel(readl(CKEN) & ~CKEN10_USBHOST, CKEN);

	return;
}
#endif

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
}

