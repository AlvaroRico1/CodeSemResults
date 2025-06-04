nfs3_proc_rmdir(struct inode *dir, const struct qstr *name)
{
	struct nfs_fattr	*dir_attr;
	struct nfs3_diropargs	arg = {
		.fh		= NFS_FH(dir),
		.name		= name->name,
		.len		= name->len
	};
	struct rpc_message msg = {
		.rpc_proc	= &nfs3_procedures[NFS3PROC_RMDIR],
		.rpc_argp	= &arg,
	};
	int status = -ENOMEM;

	dprintk("NFS call  rmdir %s\n", name->name);
	dir_attr = nfs_alloc_fattr();
	if (dir_attr == NULL)
		goto out;

	msg.rpc_resp = dir_attr;
	status = rpc_call_sync(NFS_CLIENT(dir), &msg, 0);
	nfs_post_op_update_inode(dir, dir_attr);
	nfs_free_fattr(dir_attr);
out:
	dprintk("NFS reply rmdir: %d\n", status);
	return status;
}


// Source: nfs3proc.c
// Lines 572-598
