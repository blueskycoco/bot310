/*
 * Marvell Zylonite PXA300 support file
 *
 * Copyright (C) 2010-2011 Marek Vasut <marek.vasut@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init (void)
{
	/* memory and cpu-speed are setup before relocation */
	/* so we do _nothing_ here */

	/* arch number of Lubbock-Board */
	gd->bd->bi_arch_number = MACH_TYPE_ZYLONITE;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	/* MMC */
	writel(0xa0c0, GPIO3_2);
	writel(0x141, GPIO4_2);
	writel(0x141, GPIO5_2);
	writel(0x141, GPIO6_2);
	writel(0x1c4, GPIO3);
	writel(0x1c4, GPIO4);
	writel(0x1c4, GPIO5);
	writel(0x1c4, GPIO6);
	writel(0x1c4, GPIO7);
	writel(0x1c4, GPIO8);

	/* SMC config */
	writel(0x7ff07ff0, MSC0);
	writel(0x003c7ff8, MSC1);
	writel(0x0032091d, CSADRCFG2);
	writel(0x0012c80b, CSADRCFG3);
	writel(0x2, CSMSADRCFG);
	writel(0x801, GPIO2);

	return 0;
}

int board_late_init(void)
{
	setenv("stdout", "serial");
	setenv("stderr", "serial");
	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC91111
	rc = smc91111_initialize(0, CONFIG_SMC91111_BASE);
#endif
	return rc;
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
