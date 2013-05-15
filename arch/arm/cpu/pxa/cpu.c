/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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

/*
 * CPU specific code
 */

#include <asm/io.h>
#include <asm/system.h>
#include <command.h>
#include <common.h>
#include <asm/arch/pxa-regs.h>

static void cache_flush(void);

int cleanup_before_linux (void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * just disable everything that can disturb booting linux
	 */

	disable_interrupts ();

	/* turn off I-cache */
	icache_disable();
	dcache_disable();

	/* flush I-cache */
	cache_flush();

	return (0);
}

/* flush I/D-cache */
static void cache_flush (void)
{
	unsigned long i = 0;

	asm ("mcr p15, 0, %0, c7, c5, 0": :"r" (i));
}

#ifndef CONFIG_CPU_MONAHANS
void set_GPIO_mode(int gpio_mode)
{
	int gpio = gpio_mode & GPIO_MD_MASK_NR;
	int fn = (gpio_mode & GPIO_MD_MASK_FN) >> 8;
	int val;

	/* This below changes direction setting of GPIO "gpio" */
	val = readl(GPDR(gpio));

	if (gpio_mode & GPIO_MD_MASK_DIR)
		val |= GPIO_bit(gpio);
	else
		val &= ~GPIO_bit(gpio);

	writel(val, GPDR(gpio));

	/* This below updates only AF of GPIO "gpio" */
	val = readl(GAFR(gpio));
	val &= ~(0x3 << (((gpio) & 0xf) * 2));
	val |= fn << (((gpio) & 0xf) * 2);
	writel(val, GAFR(gpio));
}
#endif /* CONFIG_CPU_MONAHANS */

void pxa_wait_ticks(int ticks)
{
	writel(0, OSCR);
	while (readl(OSCR) < ticks)
		asm volatile("":::"memory");
}

inline void writelrb(uint32_t val, uint32_t addr)
{
	writel(val, addr);
	asm volatile("":::"memory");
	readl(addr);
	asm volatile("":::"memory");
}

#ifndef	CONFIG_CPU_MONAHANS
void pxa_dram_init(void)
{
	uint32_t tmp;
	int i;
	/*
	 * 1) Initialize Asynchronous static memory controller
	 */

	writelrb(CONFIG_SYS_MSC0_VAL, MSC0);
	writelrb(CONFIG_SYS_MSC1_VAL, MSC1);
	writelrb(CONFIG_SYS_MSC2_VAL, MSC2);
	/*
	 * 2) Initialize Card Interface
	 */

	/* MECR: Memory Expansion Card Register */
	writelrb(CONFIG_SYS_MECR_VAL, MECR);
	/* MCMEM0: Card Interface slot 0 timing */
	writelrb(CONFIG_SYS_MCMEM0_VAL, MCMEM0);
	/* MCMEM1: Card Interface slot 1 timing */
	writelrb(CONFIG_SYS_MCMEM1_VAL, MCMEM1);
	/* MCATT0: Card Interface Attribute Space Timing, slot 0 */
	writelrb(CONFIG_SYS_MCATT0_VAL, MCATT0);
	/* MCATT1: Card Interface Attribute Space Timing, slot 1 */
	writelrb(CONFIG_SYS_MCATT1_VAL, MCATT1);
	/* MCIO0: Card Interface I/O Space Timing, slot 0 */
	writelrb(CONFIG_SYS_MCIO0_VAL, MCIO0);
	/* MCIO1: Card Interface I/O Space Timing, slot 1 */
	writelrb(CONFIG_SYS_MCIO1_VAL, MCIO1);

	/*
	 * 3) Configure Fly-By DMA register
	 */

	writelrb(CONFIG_SYS_FLYCNFG_VAL, FLYCNFG);

	/*
	 * 4) Initialize Timing for Sync Memory (SDCLK0)
	 */

	/*
	 * Before accessing MDREFR we need a valid DRI field, so we set
	 * this to power on defaults + DRI field.
	 */

	/* Read current MDREFR config and zero out DRI */
	tmp = readl(MDREFR) & ~0xfff;
	/* Add user-specified DRI */
	tmp |= CONFIG_SYS_MDREFR_VAL & 0xfff;
	/* Configure important bits */
	tmp |= MDREFR_K0RUN | MDREFR_SLFRSH;
	tmp &= ~(MDREFR_APD | MDREFR_E1PIN);

	/* Write MDREFR back */
	writelrb(tmp, MDREFR);

	/*
	 * 5) Initialize Synchronous Static Memory (Flash/Peripherals)
	 */

	/* Initialize SXCNFG register. Assert the enable bits.
	 *
	 * Write SXMRS to cause an MRS command to all enabled banks of
	 * synchronous static memory. Note that SXLCR need not be written
	 * at this time.
	 */
	writelrb(CONFIG_SYS_SXCNFG_VAL, SXCNFG);

	/*
	 * 6) Initialize SDRAM
	 */

	writelrb(CONFIG_SYS_MDREFR_VAL & ~MDREFR_SLFRSH, MDREFR);
	writelrb(CONFIG_SYS_MDREFR_VAL | MDREFR_E1PIN, MDREFR);

	/*
	 * 7) Write MDCNFG with MDCNFG:DEx deasserted (set to 0), to configure
	 *    but not enable each SDRAM partition pair.
	 */

	writelrb(CONFIG_SYS_MDCNFG_VAL &
		~(MDCNFG_DE0 | MDCNFG_DE1 | MDCNFG_DE2 | MDCNFG_DE3), MDCNFG);
	/* Wait for the clock to the SDRAMs to stabilize, 100..200 usec. */
	pxa_wait_ticks(0x300);

	/*
	 * 8) Trigger a number (usually 8) refresh cycles by attempting
	 *    non-burst read or write accesses to disabled SDRAM, as commonly
	 *    specified in the power up sequence documented in SDRAM data
	 *    sheets. The address(es) used for this purpose must not be
	 *    cacheable.
	 */
	for (i = 9; i >= 0; i--) {
		writel(i, 0xa0000000);
		asm volatile("":::"memory");
	}
	/*
	 * 9) Write MDCNFG with enable bits asserted (MDCNFG:DEx set to 1).
	 */

	tmp = CONFIG_SYS_MDCNFG_VAL &
		(MDCNFG_DE0 | MDCNFG_DE1 | MDCNFG_DE2 | MDCNFG_DE3);
	tmp |= readl(MDCNFG);
	writelrb(tmp, MDCNFG);

	/*
	 * 10) Write MDMRS.
	 */

	writelrb(CONFIG_SYS_MDMRS_VAL, MDMRS);

	/*
	 * 11) Enable APD
	 */

	if (CONFIG_SYS_MDREFR_VAL & MDREFR_APD) {
		tmp = readl(MDREFR);
		tmp |= MDREFR_APD;
		writelrb(tmp, MDREFR);
	}
}

void pxa_gpio_setup(void)
{
	writel(CONFIG_SYS_GPSR0_VAL, GPSR0);
	writel(CONFIG_SYS_GPSR1_VAL, GPSR1);
	writel(CONFIG_SYS_GPSR2_VAL, GPSR2);
#if defined(CONFIG_PXA27X)
	writel(CONFIG_SYS_GPSR3_VAL, GPSR3);
#endif

	writel(CONFIG_SYS_GPCR0_VAL, GPCR0);
	writel(CONFIG_SYS_GPCR1_VAL, GPCR1);
	writel(CONFIG_SYS_GPCR2_VAL, GPCR2);
#if defined(CONFIG_PXA27X)
	writel(CONFIG_SYS_GPCR3_VAL, GPCR3);
#endif

	writel(CONFIG_SYS_GPDR0_VAL, GPDR0);
	writel(CONFIG_SYS_GPDR1_VAL, GPDR1);
	writel(CONFIG_SYS_GPDR2_VAL, GPDR2);
#if defined(CONFIG_PXA27X)
	writel(CONFIG_SYS_GPDR3_VAL, GPDR3);
#endif

	writel(CONFIG_SYS_GAFR0_L_VAL, GAFR0_L);
	writel(CONFIG_SYS_GAFR0_U_VAL, GAFR0_U);
	writel(CONFIG_SYS_GAFR1_L_VAL, GAFR1_L);
	writel(CONFIG_SYS_GAFR1_U_VAL, GAFR1_U);
	writel(CONFIG_SYS_GAFR2_L_VAL, GAFR2_L);
	writel(CONFIG_SYS_GAFR2_U_VAL, GAFR2_U);
#if defined(CONFIG_PXA27X)
	writel(CONFIG_SYS_GAFR3_L_VAL, GAFR3_L);
	writel(CONFIG_SYS_GAFR3_U_VAL, GAFR3_U);
#endif

	writel(CONFIG_SYS_PSSR_VAL, PSSR);
}

void pxa_interrupt_setup(void)
{
	writel(0, ICLR);
	writel(0, ICMR);
#if defined(CONFIG_PXA27X) || defined(CONFIG_CPU_MONAHANS)
	writel(0, ICLR2);
	writel(0, ICMR2);
#endif
}

void pxa_clock_setup(void)
{
#ifndef CONFIG_CPU_MONAHANS
	writel(CONFIG_SYS_CKEN, CKEN);
	writel(CONFIG_SYS_CCCR, CCCR);
	asm volatile("mcr	p14, 0, %0, c6, c0, 0"::"r"(2));
#else
/* Set CKENA/CKENB/ACCR for MH */
#endif

	/* enable the 32Khz oscillator for RTC and PowerManager */
	writel(OSCC_OON, OSCC);
	while(!(readl(OSCC) & OSCC_OOK))
		asm volatile("":::"memory");
}

void pxa_wakeup(void)
{
	uint32_t rcsr;

	rcsr = readl(RCSR);
	writel(rcsr & (RCSR_GPR | RCSR_SMR | RCSR_WDR | RCSR_HWR), RCSR);

	/* Wakeup */
	if (rcsr & RCSR_SMR) {
		writel(PSSR_PH, PSSR);
		pxa_dram_init();
		icache_disable();
		dcache_disable();
		asm volatile("mov	pc, %0"::"r"(readl(PSSR)));
	}
}

int arch_cpu_init(void)
{
	pxa_gpio_setup();
/*	pxa_wait_ticks(0x8000); */
	pxa_wakeup();
	pxa_interrupt_setup();
	pxa_clock_setup();
	return 0;
}
#else	/* MONAHANS */

#define HSS_104M	(0)
#define HSS_156M	(1)
#define HSS_208M	(2)
#define HSS_312M	(3)

#define SMCFS_78M	(0)
#define SMCFS_104M	(2)
#define SMCFS_208M	(5)

#define SFLFS_104M	(0)
#define SFLFS_156M	(1)
#define SFLFS_208M	(2)
#define SFLFS_312M	(3)

#define XSPCLK_156M	(0)
#define XSPCLK_NONE	(3)

#define DMCFS_26M	(0)
#define DMCFS_260M	(3)

struct pxa3xx_freq_info {
	unsigned int cpufreq_mhz;
	unsigned int core_xl : 5;
	unsigned int core_xn : 3;
	unsigned int hss : 2;
	unsigned int dmcfs : 2;
	unsigned int smcfs : 3;
	unsigned int sflfs : 2;
	unsigned int df_clkdiv : 3;
};

#define OP(cpufreq, _xl, _xn, _hss, _dmc, _smc, _sfl, _dfi)		\
{									\
	.cpufreq_mhz	= cpufreq,					\
	.core_xl	= _xl,						\
	.core_xn	= _xn,						\
	.hss		= HSS_##_hss##M,				\
	.dmcfs		= DMCFS_##_dmc##M,				\
	.smcfs		= SMCFS_##_smc##M,				\
	.sflfs		= SFLFS_##_sfl##M,				\
	.df_clkdiv	= _dfi,						\
}

#if defined(CONFIG_CPU_PXA300) || defined(CONFIG_CPU_PXA310)
static struct pxa3xx_freq_info pxa300_freqs[] = {
	/*  CPU XL XN  HSS DMEM SMEM SRAM DFI VCC_CORE VCC_SRAM */
	OP(104,  8, 1, 104, 260,  78, 104, 3), /* 104MHz */
	OP(208, 16, 1, 104, 260, 104, 156, 2), /* 208MHz */
	OP(416, 16, 2, 156, 260, 104, 208, 2), /* 416MHz */
	OP(624, 24, 2, 208, 260, 208, 312, 3), /* 624MHz */
};
#endif

#if defined(CONFIG_CPU_PXA320)
static struct pxa3xx_freq_info pxa320_freqs[] = {
	/*  CPU XL XN  HSS DMEM SMEM SRAM DFI VCC_CORE VCC_SRAM */
	OP(104,  8, 1, 104, 260,  78, 104, 3), /* 104MHz */
	OP(208, 16, 1, 104, 260, 104, 156, 2), /* 208MHz */
	OP(416, 16, 2, 156, 260, 104, 208, 2), /* 416MHz */
	OP(624, 24, 2, 208, 260, 208, 312, 3), /* 624MHz */
	OP(806, 31, 2, 208, 260, 208, 312, 3), /* 806MHz */
};
#endif

void pxa_clock_setup(void)
{
	uint32_t mask = ACCR_XN_MASK | ACCR_XL_MASK;
	uint32_t accr = readl(ACCR);
	uint32_t xclkcfg;
	struct pxa3xx_freq_info *info;
#ifdef	CONFIG_PXA3XX_CPUFREQ_AUTO
#ifdef	CONFIG_CPU_PXA320
	info = &pxa320_freqs[4];
#else
	unsigned long cpuid;
	/* 624MHz for PXA310 ; 208MHz for PXA300 */
	asm volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(cpuid));
	info = &pxa300_freqs[(cpuid & 0x10) ? 3 : 1];
#endif
#else
#ifdef	CONFIG_CPU_PXA320
	info = &pxa320_freqs[CONFIG_PXA3XX_FREQ_IDX];
#else
	info = &pxa300_freqs[CONFIG_PXA3XX_FREQ_IDX];
#endif
#endif
	/* Setup CPU frequency */

	accr &= ~(ACCR_XN_MASK | ACCR_XL_MASK | ACCR_XSPCLK_MASK);
	accr |= ACCR_XN(info->core_xn) | ACCR_XL(info->core_xl);

	/* No clock until core PLL is re-locked */
	accr |= ACCR_XSPCLK(XSPCLK_NONE);

	xclkcfg = (info->core_xn == 2) ? 0x3 : 0x2;     /* turbo bit */

	writel(accr, ACCR);
	__asm__("mcr p14, 0, %0, c6, c0, 0\n" : : "r"(xclkcfg));

	while ((readl(ACSR) & mask) != (accr & mask));

	/* Setup BUS frequency */

	accr = readl(accr);
	mask = ACCR_SMCFS_MASK | ACCR_SFLFS_MASK | ACCR_HSS_MASK | ACCR_DMCFS_MASK;

	accr &= ~mask;
	accr |= ACCR_SMCFS(info->smcfs) | ACCR_SFLFS(info->sflfs) |
		ACCR_HSS(info->hss) | ACCR_DMCFS(info->dmcfs);

	writel(accr, ACCR);

	while ((readl(ACSR) & mask) != (accr & mask));
}

inline int arch_cpu_init(void)
{
	pxa_clock_setup();
	return 0;
}
#endif	/* CONFIG_CPU_MONAHANS */
