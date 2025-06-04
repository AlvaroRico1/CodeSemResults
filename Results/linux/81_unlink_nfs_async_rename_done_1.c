static void nfs_async_rename_done(struct rpc_task *task, void *calldata)
{
	struct nfs_renamedata *data = calldata;
	struct inode *old_dir = data->old_dir;
	struct inode *new_dir = data->new_dir;
	struct dentry *old_dentry = data->old_dentry;

	trace_nfs_sillyrename_rename(old_dir, old_dentry,
			new_dir, data->new_dentry, task->tk_status);
	if (!NFS_PROTO(old_dir)->rename_done(task, old_dir, new_dir)) {
		rpc_restart_call_prepare(task);
		return;
	}

	if (data->complete)
		data->complete(task, data);
}


// Source: unlink.c
// Lines 258-274
