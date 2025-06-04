static void nfs4_proc_unlink_setup(struct rpc_message *msg,
		struct dentry *dentry,
		struct inode *inode)
{
	struct nfs_removeargs *args = msg->rpc_argp;
	struct nfs_removeres *res = msg->rpc_resp;

	res->server = NFS_SB(dentry->d_sb);
	msg->rpc_proc = &nfs4_procedures[NFSPROC4_CLNT_REMOVE];
	nfs4_init_sequence(&args->seq_args, &res->seq_res, 1, 0);

	nfs_fattr_init(res->dir_attr);

	if (inode)
		nfs4_inode_return_delegation(inode);
}


// Source: nfs4proc.c
// Lines 4471-4486
