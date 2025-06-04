nfs4_proc_setattr(struct dentry *dentry, struct nfs_fattr *fattr,
		  struct iattr *sattr)
{
	struct inode *inode = d_inode(dentry);
	const struct cred *cred = NULL;
	struct nfs_open_context *ctx = NULL;
	struct nfs4_label *label = NULL;
	int status;

	if (pnfs_ld_layoutret_on_setattr(inode) &&
	    sattr->ia_valid & ATTR_SIZE &&
	    sattr->ia_size < i_size_read(inode))
		pnfs_commit_and_return_layout(inode);

	nfs_fattr_init(fattr);
	
	/* Deal with open(O_TRUNC) */
	if (sattr->ia_valid & ATTR_OPEN)
		sattr->ia_valid &= ~(ATTR_MTIME|ATTR_CTIME);

	/* Optimization: if the end result is no change, don't RPC */
	if ((sattr->ia_valid & ~(ATTR_FILE|ATTR_OPEN)) == 0)
		return 0;

	/* Search for an existing open(O_WRITE) file */
	if (sattr->ia_valid & ATTR_FILE) {

		ctx = nfs_file_open_context(sattr->ia_file);
		if (ctx)
			cred = ctx->cred;
	}

	label = nfs4_label_alloc(NFS_SERVER(inode), GFP_KERNEL);
	if (IS_ERR(label))
		return PTR_ERR(label);

	/* Return any delegations if we're going to change ACLs */
	if ((sattr->ia_valid & (ATTR_MODE|ATTR_UID|ATTR_GID)) != 0)
		nfs4_inode_make_writeable(inode);

	status = nfs4_do_setattr(inode, cred, fattr, sattr, ctx, NULL, label);
	if (status == 0) {
		nfs_setattr_update_inode(inode, sattr, fattr);
		nfs_setsecurity(inode, fattr, label);
	}
	nfs4_label_free(label);
	return status;
}


// Source: nfs4proc.c
// Lines 4050-4097
