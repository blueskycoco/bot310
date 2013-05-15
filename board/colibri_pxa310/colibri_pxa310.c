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

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init (void)
{
	/* arch number for linux kernel */
	gd->bd->bi_arch_number = MACH_TYPE_COLIBRI300;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	/* configuration for ethernet chip */
	writel(0x1c01, GPIO1);
	writel(0x7ff07ff0, MSC0);
	writel(0x7ff80779, MSC1);
	writel(0x00020000, CSADRCFG0);
	writel(0x00020000, CSADRCFG1);
	writel(0x0032c809, CSADRCFG2);
	writel(0x00020000, CSADRCFG3);
	writel(2, CSMSADRCFG);

	/* MMC */
	writel(0x804, GPIO3);
	writel(0x804, GPIO4);
	writel(0x804, GPIO5);
	writel(0x804, GPIO6);
	writel(0x804, GPIO7);
	writel(0x805, GPIO14);

	/* USB host */
	writel(0x801, GPIO0_2);
	writel(0x801, GPIO1_2);

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
	uint32_t cpuid;

	gd->ram_size = PHYS_SDRAM_1_SIZE;

	/* Check if CPU is PXA310. PXA310 colibri has twice as much RAM */
	asm volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(cpuid));
	if (cpuid & 0x10)	/* PXA310 */
		gd->ram_size <<= 1;

	return 0;
}

void dram_init_banksize(void)
{
	uint32_t cpuid;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	/* Check if CPU is PXA310. PXA310 colibri has twice as much RAM */
	asm volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(cpuid));
	if (cpuid & 0x10)	/* PXA310 */
		gd->bd->bi_dram[0].size <<= 1;
}
