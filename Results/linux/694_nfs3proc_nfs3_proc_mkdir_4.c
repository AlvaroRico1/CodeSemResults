nfs3_proc_mkdir(struct inode *dir, struct dentry *dentry, struct iattr *sattr)
{
	struct posix_acl *default_acl, *acl;
	struct nfs3_createdata *data;
	int status = -ENOMEM;

	dprintk("NFS call  mkdir %pd\n", dentry);

	data = nfs3_alloc_createdata();
	if (data == NULL)
		goto out;

	status = posix_acl_create(dir, &sattr->ia_mode, &default_acl, &acl);
	if (status)
		goto out;

	data->msg.rpc_proc = &nfs3_procedures[NFS3PROC_MKDIR];
	data->arg.mkdir.fh = NFS_FH(dir);
	data->arg.mkdir.name = dentry->d_name.name;
	data->arg.mkdir.len = dentry->d_name.len;
	data->arg.mkdir.sattr = sattr;

	status = nfs3_do_create(dir, dentry, data);
	if (status != 0)
		goto out_release_acls;

	status = nfs3_proc_setacls(d_inode(dentry), acl, default_acl);

out_release_acls:
	posix_acl_release(acl);
	posix_acl_release(default_acl);
out:
	nfs3_free_createdata(data);
	dprintk("NFS reply mkdir: %d\n", status);
	return status;
}


// Source: nfs3proc.c
// Lines 534-569
