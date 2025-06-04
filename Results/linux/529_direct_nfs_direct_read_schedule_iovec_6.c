static ssize_t nfs_direct_read_schedule_iovec(struct nfs_direct_req *dreq,
					      struct iov_iter *iter,
					      loff_t pos)
{
	struct nfs_pageio_descriptor desc;
	struct inode *inode = dreq->inode;
	ssize_t result = -EINVAL;
	size_t requested_bytes = 0;
	size_t rsize = max_t(size_t, NFS_SERVER(inode)->rsize, PAGE_SIZE);

	nfs_pageio_init_read(&desc, dreq->inode, false,
			     &nfs_direct_read_completion_ops);
	get_dreq(dreq);
	desc.pg_dreq = dreq;
	inode_dio_begin(inode);

	while (iov_iter_count(iter)) {
		struct page **pagevec;
		size_t bytes;
		size_t pgbase;
		unsigned npages, i;

		result = iov_iter_get_pages_alloc(iter, &pagevec, 
						  rsize, &pgbase);
		if (result < 0)
			break;
	
		bytes = result;
		iov_iter_advance(iter, bytes);
		npages = (result + pgbase + PAGE_SIZE - 1) / PAGE_SIZE;
		for (i = 0; i < npages; i++) {
			struct nfs_page *req;
			unsigned int req_len = min_t(size_t, bytes, PAGE_SIZE - pgbase);
			/* XXX do we need to do the eof zeroing found in async_filler? */
			req = nfs_create_request(dreq->ctx, pagevec[i],
						 pgbase, req_len);
			if (IS_ERR(req)) {
				result = PTR_ERR(req);
				break;
			}
			req->wb_index = pos >> PAGE_SHIFT;
			req->wb_offset = pos & ~PAGE_MASK;
			if (!nfs_pageio_add_request(&desc, req)) {
				result = desc.pg_error;
				nfs_release_request(req);
				break;
			}
			pgbase = 0;
			bytes -= req_len;
			requested_bytes += req_len;
			pos += req_len;
			dreq->bytes_left -= req_len;
		}
		nfs_direct_release_pages(pagevec, npages);
		kvfree(pagevec);
		if (result < 0)
			break;
	}

	nfs_pageio_complete(&desc);

	/*
	 * If no bytes were started, return the error, and let the
	 * generic layer handle the completion.
	 */
	if (requested_bytes == 0) {
		inode_dio_end(inode);
		nfs_direct_req_release(dreq);
		return result < 0 ? result : -EIO;
	}

	if (put_dreq(dreq))
		nfs_direct_complete(dreq);
	return requested_bytes;
}


// Source: direct.c
// Lines 468-542
