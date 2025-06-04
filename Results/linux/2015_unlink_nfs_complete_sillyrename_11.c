nfs_complete_sillyrename(struct rpc_task *task, struct nfs_renamedata *data)
{
	struct dentry *dentry = data->old_dentry;

	if (task->tk_status != 0) {
		nfs_cancel_async_unlink(dentry);
		return;
	}
}


// Source: unlink.c
// Lines 391-399
