static void nfs_direct_read_completion(struct nfs_pgio_header *hdr)
{
	unsigned long bytes = 0;
	struct nfs_direct_req *dreq = hdr->dreq;

	spin_lock(&dreq->lock);
	if (test_bit(NFS_IOHDR_ERROR, &hdr->flags))
		dreq->error = hdr->error;

	if (test_bit(NFS_IOHDR_REDO, &hdr->flags)) {
		spin_unlock(&dreq->lock);
		goto out_put;
	}

	if (hdr->good_bytes != 0)
		nfs_direct_good_bytes(dreq, hdr);

	if (test_bit(NFS_IOHDR_EOF, &hdr->flags))
		dreq->error = 0;

	spin_unlock(&dreq->lock);

	while (!list_empty(&hdr->pages)) {
		struct nfs_page *req = nfs_list_entry(hdr->pages.next);
		struct page *page = req->wb_page;

		if (!PageCompound(page) && bytes < hdr->good_bytes &&
		    (dreq->flags == NFS_ODIRECT_SHOULD_DIRTY))
			set_page_dirty(page);
		bytes += req->wb_bytes;
		nfs_list_remove_request(req);
		nfs_release_request(req);
	}
out_put:
	if (put_dreq(dreq))
		nfs_direct_complete(dreq);
	hdr->release(hdr);
}


// Source: direct.c
// Lines 399-436
