nfs_get_parent(struct dentry *dentry)
{
	int ret;
	struct inode *inode = d_inode(dentry), *pinode;
	struct super_block *sb = inode->i_sb;
	struct nfs_server *server = NFS_SB(sb);
	struct nfs_fattr *fattr = NULL;
	struct nfs4_label *label = NULL;
	struct dentry *parent;
	struct nfs_rpc_ops const *ops = server->nfs_client->rpc_ops;
	struct nfs_fh fh;

	if (!ops->lookupp)
		return ERR_PTR(-EACCES);

	fattr = nfs_alloc_fattr();
	if (fattr == NULL) {
		parent = ERR_PTR(-ENOMEM);
		goto out;
	}

	label = nfs4_label_alloc(server, GFP_KERNEL);
	if (IS_ERR(label)) {
		parent = ERR_CAST(label);
		goto out_free_fattr;
	}

	ret = ops->lookupp(inode, &fh, fattr, label);
	if (ret) {
		parent = ERR_PTR(ret);
		goto out_free_label;
	}

	pinode = nfs_fhget(sb, &fh, fattr, label);
	parent = d_obtain_alias(pinode);
out_free_label:
	nfs4_label_free(label);
out_free_fattr:
	nfs_free_fattr(fattr);
out:
	return parent;
}


// Source: export.c
// Lines 126-167
