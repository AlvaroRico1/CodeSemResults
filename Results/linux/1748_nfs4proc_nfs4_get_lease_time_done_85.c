static void nfs4_get_lease_time_done(struct rpc_task *task, void *calldata)
{
	struct nfs4_get_lease_time_data *data =
			(struct nfs4_get_lease_time_data *)calldata;

	dprintk("--> %s\n", __func__);
	if (!nfs4_sequence_done(task, &data->res->lr_seq_res))
		return;
	switch (task->tk_status) {
	case -NFS4ERR_DELAY:
	case -NFS4ERR_GRACE:
		dprintk("%s Retry: tk_status %d\n", __func__, task->tk_status);
		rpc_delay(task, NFS4_POLL_RETRY_MIN);
		task->tk_status = 0;
		/* fall through */
	case -NFS4ERR_RETRY_UNCACHED_REP:
		rpc_restart_call_prepare(task);
		return;
	}
	dprintk("<-- %s\n", __func__);
}


// Source: nfs4proc.c
// Lines 8318-8338
