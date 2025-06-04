static int nfs4_commit_done_cb(struct rpc_task *task, struct nfs_commit_data *data)
{
	struct inode *inode = data->inode;

	trace_nfs4_commit(data, task->tk_status);
	if (nfs4_async_handle_error(task, NFS_SERVER(inode),
				    NULL, NULL) == -EAGAIN) {
		rpc_restart_call_prepare(task);
		return -EAGAIN;
	}
	return 0;
}


// Source: nfs4proc.c
// Lines 5247-5258
