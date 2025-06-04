static int single_release_net(struct inode *ino, struct file *f)
{
	struct seq_file *seq = f->private_data;
	put_net(seq->private);
	return single_release(ino, f);
}


// Source: proc_net.c
// Lines 179-184
