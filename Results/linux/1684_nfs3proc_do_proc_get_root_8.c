do_proc_get_root(struct rpc_clnt *client, struct nfs_fh *fhandle,
		 struct nfs_fsinfo *info)
{
	struct rpc_message msg = {
		.rpc_proc	= &nfs3_procedures[NFS3PROC_FSINFO],
		.rpc_argp	= fhandle,
		.rpc_resp	= info,
	};
	int	status;

	dprintk("%s: call  fsinfo\n", __func__);
	nfs_fattr_init(info->fattr);
	status = rpc_call_sync(client, &msg, 0);
	dprintk("%s: reply fsinfo: %d\n", __func__, status);
	if (status == 0 && !(info->fattr->valid & NFS_ATTR_FATTR)) {
		msg.rpc_proc = &nfs3_procedures[NFS3PROC_GETATTR];
		msg.rpc_resp = info->fattr;
		status = rpc_call_sync(client, &msg, 0);
		dprintk("%s: reply getattr: %d\n", __func__, status);
	}
	return status;
}


// Source: nfs3proc.c
// Lines 61-82
