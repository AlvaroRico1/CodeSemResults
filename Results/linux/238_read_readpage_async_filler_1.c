readpage_async_filler(void *data, struct page *page)
{
	struct nfs_readdesc *desc = (struct nfs_readdesc *)data;
	struct nfs_page *new;
	unsigned int len;
	int error;

	len = nfs_page_length(page);
	if (len == 0)
		return nfs_return_empty_page(page);

	new = nfs_create_request(desc->ctx, page, 0, len);
	if (IS_ERR(new))
		goto out_error;

	if (len < PAGE_SIZE)
		zero_user_segment(page, len, PAGE_SIZE);
	if (!nfs_pageio_add_request(desc->pgio, new)) {
		nfs_list_remove_request(new);
		error = desc->pgio->pg_error;
		nfs_readpage_release(new, error);
		goto out;
	}
	return 0;
out_error:
	error = PTR_ERR(new);
	unlock_page(page);
out:
	return error;
}


// Source: read.c
// Lines 373-402
