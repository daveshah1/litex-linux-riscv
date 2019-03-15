/*
 * Copyright (C) 2009 Chen Liqin <liqin.chen@sunplusct.com>
 * Copyright (C) 2012 Regents of the University of California
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 */

#ifndef _ASM_RISCV_TLBFLUSH_H
#define _ASM_RISCV_TLBFLUSH_H

#include <linux/mm_types.h>

#include <utils/puts.h>

/*
 * Flush entire local TLB.  'sfence.vma' implicitly fences with the instruction
 * cache as well, so a 'fence.i' is not necessary.
 */

extern void vexriscv_mmu_map(uint32_t virt, uint32_t phys_and_flags, uint32_t location);
extern void vexriscv_dcache_clear(uint32_t line);


static inline void local_flush_tlb_all(void)
{
//DBGMSG("Flushing whole TLB!");
int i ;
	for (i = 0; i < 256; i++) {
		vexriscv_mmu_map(0,0,i);
	}
	for (i = 0; i < 4096; i+=32)
		vexriscv_dcache_clear(i);
//    __asm__ __volatile__ ("sfence.vma" : : : "memory"); // TODO
}

extern uint32_t shadow_tlb[256];

/* Flush one page from local TLB */
static inline void local_flush_tlb_page(unsigned long addr)
{
int i;
//DBGMSG("Kernel tried to flush 0x%X", addr);
	for ( i = 0; i < 256; i++)
		if (shadow_tlb[i] == (((addr >> 12) & 0xFFFFF)))
			vexriscv_mmu_map(0,0,i);
	for (i = 0; i < 4096; i+=32)
		vexriscv_dcache_clear(i);
//	__asm__ __volatile__ ("sfence.vma %0" : : "r" (addr) : "memory");    // TODO
}

#ifndef CONFIG_SMP

#define flush_tlb_all() local_flush_tlb_all()
#define flush_tlb_page(vma, addr) local_flush_tlb_page(addr)

static inline void flush_tlb_range(struct vm_area_struct *vma,
		unsigned long start, unsigned long end)
{
	local_flush_tlb_all();
}

#define flush_tlb_mm(mm) flush_tlb_all()

#else /* CONFIG_SMP */

#include <asm/sbi.h>

#define flush_tlb_all() sbi_remote_sfence_vma(NULL, 0, -1)
#define flush_tlb_page(vma, addr) flush_tlb_range(vma, addr, 0)
#define flush_tlb_range(vma, start, end) \
	sbi_remote_sfence_vma(mm_cpumask((vma)->vm_mm)->bits, \
			      start, (end) - (start))
#define flush_tlb_mm(mm) \
	sbi_remote_sfence_vma(mm_cpumask(mm)->bits, 0, -1)

#endif /* CONFIG_SMP */

/* Flush a range of kernel pages */
static inline void flush_tlb_kernel_range(unsigned long start,
	unsigned long end)
{
	flush_tlb_all();
}

#endif /* _ASM_RISCV_TLBFLUSH_H */
