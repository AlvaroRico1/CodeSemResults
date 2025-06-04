static bool nfs3_nlm_unlock_prepare(struct rpc_task *task, void *data)
{
	struct nfs_lock_context *l_ctx = data;
	if (l_ctx && test_bit(NFS_CONTEXT_UNLOCK, &l_ctx->open_context->flags))
		return nfs_async_iocounter_wait(task, l_ctx);
	return false;

}


// Source: nfs3proc.c
// Lines 871-878
