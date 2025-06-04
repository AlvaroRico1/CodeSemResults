nfs_page_group_destroy(struct kref *kref)
{
	struct nfs_page *req = container_of(kref, struct nfs_page, wb_kref);
	struct nfs_page *head = req->wb_head;
	struct nfs_page *tmp, *next;

	if (!nfs_page_group_sync_on_bit(req, PG_TEARDOWN))
		goto out;

	tmp = req;
	do {
		next = tmp->wb_this_page;
		/* unlink and free */
		tmp->wb_this_page = tmp;
		tmp->wb_head = tmp;
		nfs_free_request(tmp);
		tmp = next;
	} while (tmp != req);
out:
	/* subrequests must release the ref on the head request */
	if (head != req)
		nfs_release_request(head);
}


// Source: pagelist.c
// Lines 275-297
