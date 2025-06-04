int nfs_readdir_filler(void *data, struct page* page)
{
	nfs_readdir_descriptor_t *desc = data;
	struct inode	*inode = file_inode(desc->file);
	int ret;

	ret = nfs_readdir_xdr_to_array(desc, page, inode);
	if (ret < 0)
		goto error;
	SetPageUptodate(page);

	if (invalidate_inode_pages2_range(inode->i_mapping, page->index + 1, -1) < 0) {
		/* Should never happen */
		nfs_zap_mapping(inode, inode->i_mapping);
	}
	unlock_page(page);
	return 0;
 error:
	unlock_page(page);
	return ret;
}


// Source: dir.c
// Lines 667-687
