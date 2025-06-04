static void nfs_clear_request(struct nfs_page *req)
{
	struct page *page = req->wb_page;
	struct nfs_lock_context *l_ctx = req->wb_lock_context;
	struct nfs_open_context *ctx;

	if (page != NULL) {
		put_page(page);
		req->wb_page = NULL;
	}
	if (l_ctx != NULL) {
		if (atomic_dec_and_test(&l_ctx->io_count)) {
			wake_up_var(&l_ctx->io_count);
			ctx = l_ctx->open_context;
			if (test_bit(NFS_CONTEXT_UNLOCK, &ctx->flags))
				rpc_wake_up(&NFS_SERVER(d_inode(ctx->dentry))->uoc_rpcwaitq);
		}
		nfs_put_lock_context(l_ctx);
		req->wb_lock_context = NULL;
	}
}


// Source: pagelist.c
// Lines 414-434
