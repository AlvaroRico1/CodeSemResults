static void nfs_commit_done(struct rpc_task *task, void *calldata)
{
	struct nfs_commit_data	*data = calldata;

        dprintk("NFS: %5u nfs_commit_done (status %d)\n",
                                task->tk_pid, task->tk_status);

	/* Call the NFS version-specific code */
	NFS_PROTO(data->inode)->commit_done(task, data);
	trace_nfs_commit_done(data);
}


// Source: write.c
// Lines 1817-1827
