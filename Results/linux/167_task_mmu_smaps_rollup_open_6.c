static int smaps_rollup_open(struct inode *inode, struct file *file)
{
	int ret;
	struct proc_maps_private *priv;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL_ACCOUNT);
	if (!priv)
		return -ENOMEM;

	ret = single_open(file, show_smaps_rollup, priv);
	if (ret)
		goto out_free;

	priv->inode = inode;
	priv->mm = proc_mem_open(inode, PTRACE_MODE_READ);
	if (IS_ERR(priv->mm)) {
		ret = PTR_ERR(priv->mm);

		single_release(inode, file);
		goto out_free;
	}

	return 0;

out_free:
	kfree(priv);
	return ret;
}


// Source: task_mmu.c
// Lines 911-938
