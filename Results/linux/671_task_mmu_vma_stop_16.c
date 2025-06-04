static void vma_stop(struct proc_maps_private *priv)
{
	struct mm_struct *mm = priv->mm;

	release_task_mempolicy(priv);
	up_read(&mm->mmap_sem);
	mmput(mm);
}


// Source: task_mmu.c
// Lines 126-133
