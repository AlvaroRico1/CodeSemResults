static void nfs4_proc_rename_setup(struct rpc_message *msg,
		struct dentry *old_dentry,
		struct dentry *new_dentry)
{
	struct nfs_renameargs *arg = msg->rpc_argp;
	struct nfs_renameres *res = msg->rpc_resp;
	struct inode *old_inode = d_inode(old_dentry);
	struct inode *new_inode = d_inode(new_dentry);

	if (old_inode)
		nfs4_inode_make_writeable(old_inode);
	if (new_inode)
		nfs4_inode_return_delegation(new_inode);
	msg->rpc_proc = &nfs4_procedures[NFSPROC4_CLNT_RENAME];
	res->server = NFS_SB(old_dentry->d_sb);
	nfs4_init_sequence(&arg->seq_args, &res->seq_res, 1, 0);
}


// Source: nfs4proc.c
// Lines 4512-4528
