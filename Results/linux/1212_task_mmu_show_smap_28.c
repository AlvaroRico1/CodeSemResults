static int show_smap(struct seq_file *m, void *v)
{
	struct vm_area_struct *vma = v;
	struct mem_size_stats mss;

	memset(&mss, 0, sizeof(mss));

	smap_gather_stats(vma, &mss);

	show_map_vma(m, vma);

	SEQ_PUT_DEC("Size:           ", vma->vm_end - vma->vm_start);
	SEQ_PUT_DEC(" kB\nKernelPageSize: ", vma_kernel_pagesize(vma));
	SEQ_PUT_DEC(" kB\nMMUPageSize:    ", vma_mmu_pagesize(vma));
	seq_puts(m, " kB\n");

	__show_smap(m, &mss, false);

	seq_printf(m, "THPeligible:		%d\n",
		   transparent_hugepage_enabled(vma));

	if (arch_pkeys_enabled())
		seq_printf(m, "ProtectionKey:  %8u\n", vma_pkey(vma));
	show_smap_vma_flags(m, vma);

	m_cache_vma(m, vma);

	return 0;
}


// Source: task_mmu.c
// Lines 817-845
