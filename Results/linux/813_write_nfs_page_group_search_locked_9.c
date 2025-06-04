nfs_page_group_search_locked(struct nfs_page *head, unsigned int page_offset)
{
	struct nfs_page *req;

	req = head;
	do {
		if (page_offset >= req->wb_pgbase &&
		    page_offset < (req->wb_pgbase + req->wb_bytes))
			return req;

		req = req->wb_this_page;
	} while (req != head);

	return NULL;
}


// Source: write.c
// Lines 269-283
