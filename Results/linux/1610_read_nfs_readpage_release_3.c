static void nfs_readpage_release(struct nfs_page *req, int error)
{
	struct inode *inode = d_inode(nfs_req_openctx(req)->dentry);
	struct page *page = req->wb_page;

	dprintk("NFS: read done (%s/%llu %d@%lld)\n", inode->i_sb->s_id,
		(unsigned long long)NFS_FILEID(inode), req->wb_bytes,
		(long long)req_offset(req));

	if (nfs_error_is_fatal_on_server(error) && error != -ETIMEDOUT)
		SetPageError(page);
	if (nfs_page_group_sync_on_bit(req, PG_UNLOCKPAGE)) {
		struct address_space *mapping = page_file_mapping(page);

		if (PageUptodate(page))
			nfs_readpage_to_fscache(inode, page, 0);
		else if (!PageError(page) && !PagePrivate(page))
			generic_error_remove_page(mapping, page);
		unlock_page(page);
	}
	nfs_release_request(req);
}


// Source: read.c
// Lines 94-115
