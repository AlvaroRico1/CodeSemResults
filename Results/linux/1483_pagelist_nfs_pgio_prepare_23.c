static void nfs_pgio_prepare(struct rpc_task *task, void *calldata)
{
	struct nfs_pgio_header *hdr = calldata;
	int err;
	err = NFS_PROTO(hdr->inode)->pgio_rpc_prepare(task, hdr);
	if (err)
		rpc_exit(task, err);
}


// Source: pagelist.c
// Lines 604-611
