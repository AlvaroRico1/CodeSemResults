ssize_t nfs_file_write(struct kiocb *iocb, struct iov_iter *from)
{
	struct file *file = iocb->ki_filp;
	struct inode *inode = file_inode(file);
	unsigned long written = 0;
	ssize_t result;

	result = nfs_key_timeout_notify(file, inode);
	if (result)
		return result;

	if (iocb->ki_flags & IOCB_DIRECT)
		return nfs_file_direct_write(iocb, from);

	dprintk("NFS: write(%pD2, %zu@%Ld)\n",
		file, iov_iter_count(from), (long long) iocb->ki_pos);

	if (IS_SWAPFILE(inode))
		goto out_swapfile;
	/*
	 * O_APPEND implies that we must revalidate the file length.
	 */
	if (iocb->ki_flags & IOCB_APPEND) {
		result = nfs_revalidate_file_size(inode, file);
		if (result)
			goto out;
	}
	if (iocb->ki_pos > i_size_read(inode))
		nfs_revalidate_mapping(inode, file->f_mapping);

	nfs_start_io_write(inode);
	result = generic_write_checks(iocb, from);
	if (result > 0) {
		current->backing_dev_info = inode_to_bdi(inode);
		result = generic_perform_write(file, from, iocb->ki_pos);
		current->backing_dev_info = NULL;
	}
	nfs_end_io_write(inode);
	if (result <= 0)
		goto out;

	written = result;
	iocb->ki_pos += written;
	result = generic_write_sync(iocb, written);
	if (result < 0)
		goto out;

	/* Return error values */
	if (nfs_need_check_write(file, inode)) {
		int err = nfs_wb_all(inode);
		if (err < 0)
			result = err;
	}
	nfs_add_stats(inode, NFSIOS_NORMALWRITTENBYTES, written);
out:
	return result;

out_swapfile:
	printk(KERN_INFO "NFS: attempt to write to active swap file!\n");
	return -EBUSY;
}


// Source: file.c
// Lines 593-653
