int nfs_updatepage(struct file *file, struct page *page,
		unsigned int offset, unsigned int count)
{
	struct nfs_open_context *ctx = nfs_file_open_context(file);
	struct address_space *mapping = page_file_mapping(page);
	struct inode	*inode = mapping->host;
	int		status = 0;

	nfs_inc_stats(inode, NFSIOS_VFSUPDATEPAGE);

	dprintk("NFS:       nfs_updatepage(%pD2 %d@%lld)\n",
		file, count, (long long)(page_file_offset(page) + offset));

	if (!count)
		goto out;

	if (nfs_can_extend_write(file, page, inode)) {
		count = max(count + offset, nfs_page_length(page));
		offset = 0;
	}

	status = nfs_writepage_setup(ctx, page, offset, count);
	if (status < 0)
		nfs_set_pageerror(mapping);
	else
		__set_page_dirty_nobuffers(page);
out:
	dprintk("NFS:       nfs_updatepage returns %d (isize %lld)\n",
			status, (long long)i_size_read(inode));
	return status;
}


// Source: write.c
// Lines 1353-1383
