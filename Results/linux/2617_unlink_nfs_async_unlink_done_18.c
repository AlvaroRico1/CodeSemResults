static void nfs_async_unlink_done(struct rpc_task *task, void *calldata)
{
	struct nfs_unlinkdata *data = calldata;
	struct inode *dir = d_inode(data->dentry->d_parent);

	trace_nfs_sillyrename_unlink(data, task->tk_status);
	if (!NFS_PROTO(dir)->unlink_done(task, dir))
		rpc_restart_call_prepare(task);
}


// Source: unlink.c
// Lines 46-54
