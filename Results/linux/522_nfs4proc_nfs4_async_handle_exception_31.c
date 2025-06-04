nfs4_async_handle_exception(struct rpc_task *task, struct nfs_server *server,
		int errorcode, struct nfs4_exception *exception)
{
	struct nfs_client *clp = server->nfs_client;
	int ret;

	ret = nfs4_do_handle_exception(server, errorcode, exception);
	if (exception->delay) {
		rpc_delay(task, nfs4_update_delay(&exception->timeout));
		goto out_retry;
	}
	if (exception->recovering) {
		rpc_sleep_on(&clp->cl_rpcwaitq, task, NULL);
		if (test_bit(NFS4CLNT_MANAGER_RUNNING, &clp->cl_state) == 0)
			rpc_wake_up_queued_task(&clp->cl_rpcwaitq, task);
		goto out_retry;
	}
	if (test_bit(NFS_MIG_FAILED, &server->mig_status))
		ret = -EIO;
	return ret;
out_retry:
	if (ret == 0) {
		exception->retry = 1;
		/*
		 * For NFS4ERR_MOVED, the client transport will need to
		 * be recomputed after migration recovery has completed.
		 */
		if (errorcode == -NFS4ERR_MOVED)
			rpc_task_release_transport(task);
	}
	return ret;
}


// Source: nfs4proc.c
// Lines 599-630
