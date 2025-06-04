ssize_t nfs_direct_IO(struct kiocb *iocb, struct iov_iter *iter)
{
	struct inode *inode = iocb->ki_filp->f_mapping->host;

	/* we only support swap file calling nfs_direct_IO */
	if (!IS_SWAPFILE(inode))
		return 0;

	VM_BUG_ON(iov_iter_count(iter) != PAGE_SIZE);

	if (iov_iter_rw(iter) == READ)
		return nfs_file_direct_read(iocb, iter);
	return nfs_file_direct_write(iocb, iter);
}


// Source: direct.c
// Lines 264-277
