static void nfs4_lock_release(void *calldata)
{
	struct nfs4_lockdata *data = calldata;

	dprintk("%s: begin!\n", __func__);
	nfs_free_seqid(data->arg.open_seqid);
	if (data->cancelled && data->rpc_status == 0) {
		struct rpc_task *task;
		task = nfs4_do_unlck(&data->fl, data->ctx, data->lsp,
				data->arg.lock_seqid);
		if (!IS_ERR(task))
			rpc_put_task_async(task);
		dprintk("%s: cancelling lock!\n", __func__);
	} else
		nfs_free_seqid(data->arg.lock_seqid);
	nfs4_put_lock_state(data->lsp);
	put_nfs_open_context(data->ctx);
	kfree(data);
	dprintk("%s: done!\n", __func__);
}


// Source: nfs4proc.c
// Lines 6741-6760
