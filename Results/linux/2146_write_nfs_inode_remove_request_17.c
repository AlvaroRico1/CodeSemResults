static void nfs_inode_remove_request(struct nfs_page *req)
{
	struct address_space *mapping = page_file_mapping(req->wb_page);
	struct inode *inode = mapping->host;
	struct nfs_inode *nfsi = NFS_I(inode);
	struct nfs_page *head;

	atomic_long_dec(&nfsi->nrequests);
	if (nfs_page_group_sync_on_bit(req, PG_REMOVE)) {
		head = req->wb_head;

		spin_lock(&mapping->private_lock);
		if (likely(head->wb_page && !PageSwapCache(head->wb_page))) {
			set_page_private(head->wb_page, 0);
			ClearPagePrivate(head->wb_page);
			clear_bit(PG_MAPPED, &head->wb_flags);
		}
		spin_unlock(&mapping->private_lock);
	}

	if (test_and_clear_bit(PG_INODE_REF, &req->wb_flags))
		nfs_release_request(req);
}


// Source: write.c
// Lines 782-804
