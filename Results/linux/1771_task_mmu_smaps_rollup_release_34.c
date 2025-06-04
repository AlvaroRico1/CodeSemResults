static int smaps_rollup_release(struct inode *inode, struct file *file)
{
	struct seq_file *seq = file->private_data;
	struct proc_maps_private *priv = seq->private;

	if (priv->mm)
		mmdrop(priv->mm);

	kfree(priv);
	return single_release(inode, file);
}


// Source: task_mmu.c
// Lines 940-950
