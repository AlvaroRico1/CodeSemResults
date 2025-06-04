nfs_destroy_unlinked_subrequests(struct nfs_page *destroy_list,
				 struct nfs_page *old_head,
				 struct inode *inode)
{
	while (destroy_list) {
		struct nfs_page *subreq = destroy_list;

		destroy_list = (subreq->wb_this_page == old_head) ?
				   NULL : subreq->wb_this_page;

		WARN_ON_ONCE(old_head != subreq->wb_head);

		/* make sure old group is not used */
		subreq->wb_this_page = subreq;

		clear_bit(PG_REMOVE, &subreq->wb_flags);

		/* Note: races with nfs_page_group_destroy() */
		if (!kref_read(&subreq->wb_kref)) {
			/* Check if we raced with nfs_page_group_destroy() */
			if (test_and_clear_bit(PG_TEARDOWN, &subreq->wb_flags))
				nfs_free_request(subreq);
			continue;
		}

		subreq->wb_head = subreq;

		if (test_and_clear_bit(PG_INODE_REF, &subreq->wb_flags)) {
			nfs_release_request(subreq);
			atomic_long_dec(&NFS_I(inode)->nrequests);
		}

		/* subreq is now totally disconnected from page group or any
		 * write / commit lists. last chance to wake any waiters */
		nfs_unlock_and_release_request(subreq);
	}


// Source: write.c
// Lines 410-445
