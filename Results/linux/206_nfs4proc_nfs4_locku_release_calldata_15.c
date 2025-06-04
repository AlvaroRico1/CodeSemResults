static void nfs4_locku_release_calldata(void *data)
{
	struct nfs4_unlockdata *calldata = data;
	nfs_free_seqid(calldata->arg.seqid);
	nfs4_put_lock_state(calldata->lsp);
	nfs_put_lock_context(calldata->l_ctx);
	put_nfs_open_context(calldata->ctx);
	kfree(calldata);
}


// Source: nfs4proc.c
// Lines 6425-6433
