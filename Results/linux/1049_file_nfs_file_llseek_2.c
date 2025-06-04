loff_t nfs_file_llseek(struct file *filp, loff_t offset, int whence)
{
	dprintk("NFS: llseek file(%pD2, %lld, %d)\n",
			filp, offset, whence);

	/*
	 * whence == SEEK_END || SEEK_DATA || SEEK_HOLE => we must revalidate
	 * the cached file length
	 */
	if (whence != SEEK_SET && whence != SEEK_CUR) {
		struct inode *inode = filp->f_mapping->host;

		int retval = nfs_revalidate_file_size(inode, filp);
		if (retval < 0)
			return (loff_t)retval;
	}


// Source: file.c
// Lines 115-130
