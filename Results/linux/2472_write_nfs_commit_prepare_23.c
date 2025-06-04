void nfs_commit_prepare(struct rpc_task *task, void *calldata)
{
	struct nfs_commit_data *data = calldata;

	NFS_PROTO(data->inode)->commit_rpc_prepare(task, data);
}


// Source: write.c
// Lines 1484-1489
