static int nfs3_proc_readlink(struct inode *inode, struct page *page,
		unsigned int pgbase, unsigned int pglen)
{
	struct nfs_fattr	*fattr;
	struct nfs3_readlinkargs args = {
		.fh		= NFS_FH(inode),
		.pgbase		= pgbase,
		.pglen		= pglen,
		.pages		= &page
	};
	struct rpc_message msg = {
		.rpc_proc	= &nfs3_procedures[NFS3PROC_READLINK],
		.rpc_argp	= &args,
	};
	int status = -ENOMEM;

	dprintk("NFS call  readlink\n");
	fattr = nfs_alloc_fattr();
	if (fattr == NULL)
		goto out;
	msg.rpc_resp = fattr;

	status = rpc_call_sync(NFS_CLIENT(inode), &msg, 0);
	nfs_refresh_inode(inode, fattr);
	nfs_free_fattr(fattr);
out:
	dprintk("NFS reply readlink: %d\n", status);
	return status;
}


// Source: nfs3proc.c
// Lines 221-249
