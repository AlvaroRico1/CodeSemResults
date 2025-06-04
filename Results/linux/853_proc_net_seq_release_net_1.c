static int seq_release_net(struct inode *ino, struct file *f)
{
	struct seq_file *seq = f->private_data;

	put_net(seq_file_net(seq));
	seq_release_private(ino, f);
	return 0;
}


// Source: proc_net.c
// Lines 84-91
