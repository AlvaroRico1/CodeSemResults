static void nfs_unlink_prepare(struct rpc_task *task, void *calldata)
{
	struct nfs_unlinkdata *data = calldata;
	struct inode *dir = d_inode(data->dentry->d_parent);
	NFS_PROTO(dir)->unlink_rpc_prepare(task, data);
}


// Source: unlink.c
// Lines 76-81
