int uncached_readdir(nfs_readdir_descriptor_t *desc)
{
	struct page	*page = NULL;
	int		status;
	struct inode *inode = file_inode(desc->file);
	struct nfs_open_dir_context *ctx = desc->file->private_data;

	dfprintk(DIRCACHE, "NFS: uncached_readdir() searching for cookie %Lu\n",
			(unsigned long long)*desc->dir_cookie);

	page = alloc_page(GFP_HIGHUSER);
	if (!page) {
		status = -ENOMEM;
		goto out;
	}

	desc->page_index = 0;
	desc->last_cookie = *desc->dir_cookie;
	desc->page = page;
	ctx->duped = 0;

	status = nfs_readdir_xdr_to_array(desc, page, inode);
	if (status < 0)
		goto out_release;

	status = nfs_do_filldir(desc);

 out:
	dfprintk(DIRCACHE, "NFS: %s: returns %d\n",
			__func__, status);
	return status;
 out_release:
	cache_page_release(desc);
	goto out;
}


// Source: dir.c
// Lines 792-826
