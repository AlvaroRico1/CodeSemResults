static void nfs3_nlm_release_call(void *data)
{
	struct nfs_lock_context *l_ctx = data;
	struct nfs_open_context *ctx;
	if (l_ctx && test_bit(NFS_CONTEXT_UNLOCK, &l_ctx->open_context->flags)) {
		ctx = l_ctx->open_context;
		nfs_put_lock_context(l_ctx);
		put_nfs_open_context(ctx);
	}
}


// Source: nfs3proc.c
// Lines 880-889
