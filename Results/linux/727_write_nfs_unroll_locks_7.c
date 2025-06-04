nfs_unroll_locks(struct inode *inode, struct nfs_page *head,
			  struct nfs_page *req)
{
	struct nfs_page *tmp;

	/* relinquish all the locks successfully grabbed this run */
	for (tmp = head->wb_this_page ; tmp != req; tmp = tmp->wb_this_page) {
		if (!kref_read(&tmp->wb_kref))
			continue;
		nfs_unlock_and_release_request(tmp);
	}
}


// Source: write.c
// Lines 386-397
