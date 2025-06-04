static void nfs_pgio_result(struct rpc_task *task, void *calldata)
{
	struct nfs_pgio_header *hdr = calldata;
	struct inode *inode = hdr->inode;

	dprintk("NFS: %s: %5u, (status %d)\n", __func__,
		task->tk_pid, task->tk_status);

	if (hdr->rw_ops->rw_done(task, hdr, inode) != 0)
		return;
	if (task->tk_status < 0)
		nfs_set_pgio_error(hdr, task->tk_status, hdr->args.offset);
	else
		hdr->rw_ops->rw_result(task, hdr);
}


// Source: pagelist.c
// Lines 734-748
