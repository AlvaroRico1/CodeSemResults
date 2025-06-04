int nfs_do_filldir(nfs_readdir_descriptor_t *desc)
{
	struct file	*file = desc->file;
	int i = 0;
	int res = 0;
	struct nfs_cache_array *array = NULL;
	struct nfs_open_dir_context *ctx = file->private_data;

	array = kmap(desc->page);
	for (i = desc->cache_entry_index; i < array->size; i++) {
		struct nfs_cache_array_entry *ent;

		ent = &array->array[i];
		if (!dir_emit(desc->ctx, ent->string.name, ent->string.len,
		    nfs_compat_user_ino64(ent->ino), ent->d_type)) {
			desc->eof = true;
			break;
		}
		desc->ctx->pos++;
		if (i < (array->size-1))
			*desc->dir_cookie = array->array[i+1].cookie;
		else
			*desc->dir_cookie = array->last_cookie;
		if (ctx->duped != 0)
			ctx->duped = 1;
	}
	if (array->eof_index >= 0)
		desc->eof = true;

	kunmap(desc->page);
	cache_page_release(desc);
	dfprintk(DIRCACHE, "NFS: nfs_do_filldir() filling ended @ cookie %Lu; returning = %d\n",
			(unsigned long long)*desc->dir_cookie, res);
	return res;
}


// Source: dir.c
// Lines 743-777
