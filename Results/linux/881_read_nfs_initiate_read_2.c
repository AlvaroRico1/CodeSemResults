static void nfs_initiate_read(struct nfs_pgio_header *hdr,
			      struct rpc_message *msg,
			      const struct nfs_rpc_ops *rpc_ops,
			      struct rpc_task_setup *task_setup_data, int how)
{
	struct inode *inode = hdr->inode;
	int swap_flags = IS_SWAPFILE(inode) ? NFS_RPC_SWAPFLAGS : 0;

	task_setup_data->flags |= swap_flags;
	rpc_ops->read_setup(hdr, msg);
	trace_nfs_initiate_read(inode, hdr->io_start, hdr->good_bytes);
}


// Source: read.c
// Lines 207-218
