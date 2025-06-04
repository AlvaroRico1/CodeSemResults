int nfs_readdir_xdr_to_array(nfs_readdir_descriptor_t *desc, struct page *page, struct inode *inode)
{
	struct page *pages[NFS_MAX_READDIR_PAGES];
	struct nfs_entry entry;
	struct file	*file = desc->file;
	struct nfs_cache_array *array;
	int status = -ENOMEM;
	unsigned int array_size = ARRAY_SIZE(pages);

	entry.prev_cookie = 0;
	entry.cookie = desc->last_cookie;
	entry.eof = 0;
	entry.fh = nfs_alloc_fhandle();
	entry.fattr = nfs_alloc_fattr();
	entry.server = NFS_SERVER(inode);
	if (entry.fh == NULL || entry.fattr == NULL)
		goto out;

	entry.label = nfs4_label_alloc(NFS_SERVER(inode), GFP_NOWAIT);
	if (IS_ERR(entry.label)) {
		status = PTR_ERR(entry.label);
		goto out;
	}

	array = kmap(page);
	memset(array, 0, sizeof(struct nfs_cache_array));
	array->eof_index = -1;

	status = nfs_readdir_alloc_pages(pages, array_size);
	if (status < 0)
		goto out_release_array;
	do {
		unsigned int pglen;
		status = nfs_readdir_xdr_filler(pages, desc, &entry, file, inode);

		if (status < 0)
			break;
		pglen = status;
		status = nfs_readdir_page_filler(desc, &entry, pages, page, pglen);
		if (status < 0) {
			if (status == -ENOSPC)
				status = 0;
			break;
		}
	} while (array->eof_index < 0);

	nfs_readdir_free_pages(pages, array_size);
out_release_array:
	kunmap(page);
	nfs4_label_free(entry.label);
out:
	nfs_free_fattr(entry.fattr);
	nfs_free_fhandle(entry.fh);
	return status;
}


// Source: dir.c
// Lines 604-658
