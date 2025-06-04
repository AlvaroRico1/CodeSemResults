static int nfs4_write_done_cb(struct rpc_task *task,
			      struct nfs_pgio_header *hdr)
{
	struct inode *inode = hdr->inode;

	trace_nfs4_write(hdr, task->tk_status);
	if (task->tk_status < 0) {
		struct nfs4_exception exception = {
			.inode = hdr->inode,
			.state = hdr->args.context->state,
			.stateid = &hdr->args.stateid,
		};
		task->tk_status = nfs4_async_handle_exception(task,
				NFS_SERVER(inode), task->tk_status,
				&exception);
		if (exception.retry) {
			rpc_restart_call_prepare(task);
			return -EAGAIN;
		}
	}
	if (task->tk_status >= 0) {
		renew_lease(NFS_SERVER(inode), hdr->timestamp);
		nfs_writeback_update_inode(hdr);
	}
	return 0;
}


// Source: nfs4proc.c
// Lines 5154-5179
