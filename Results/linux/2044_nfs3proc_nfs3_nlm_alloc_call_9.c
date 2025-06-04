static void nfs3_nlm_alloc_call(void *data)
{
	struct nfs_lock_context *l_ctx = data;
	if (l_ctx && test_bit(NFS_CONTEXT_UNLOCK, &l_ctx->open_context->flags)) {
		get_nfs_open_context(l_ctx->open_context);
		nfs_get_lock_context(l_ctx->open_context);
	}
}


// Source: nfs3proc.c
// Lines 862-869
