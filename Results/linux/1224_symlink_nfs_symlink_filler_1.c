static int nfs_symlink_filler(void *data, struct page *page)
{
	struct inode *inode = data;
	int error;

	error = NFS_PROTO(inode)->readlink(inode, page, 0, PAGE_SIZE);
	if (error < 0)
		goto error;
	SetPageUptodate(page);
	unlock_page(page);
	return 0;

error:
	SetPageError(page);
	unlock_page(page);
	return -EIO;
}


// Source: symlink.c
// Lines 29-45
