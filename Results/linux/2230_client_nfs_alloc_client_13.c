struct nfs_client *nfs_alloc_client(const struct nfs_client_initdata *cl_init)
{
	struct nfs_client *clp;
	int err = -ENOMEM;

	if ((clp = kzalloc(sizeof(*clp), GFP_KERNEL)) == NULL)
		goto error_0;

	clp->cl_nfs_mod = cl_init->nfs_mod;
	if (!try_module_get(clp->cl_nfs_mod->owner))
		goto error_dealloc;

	clp->rpc_ops = clp->cl_nfs_mod->rpc_ops;

	refcount_set(&clp->cl_count, 1);
	clp->cl_cons_state = NFS_CS_INITING;

	memcpy(&clp->cl_addr, cl_init->addr, cl_init->addrlen);
	clp->cl_addrlen = cl_init->addrlen;

	if (cl_init->hostname) {
		err = -ENOMEM;
		clp->cl_hostname = kstrdup(cl_init->hostname, GFP_KERNEL);
		if (!clp->cl_hostname)
			goto error_cleanup;
	}

	INIT_LIST_HEAD(&clp->cl_superblocks);
	clp->cl_rpcclient = ERR_PTR(-EINVAL);

	clp->cl_proto = cl_init->proto;
	clp->cl_nconnect = cl_init->nconnect;
	clp->cl_net = get_net(cl_init->net);

	clp->cl_principal = "*";
	nfs_fscache_get_client_cookie(clp);

	return clp;

error_cleanup:
	put_nfs_version(clp->cl_nfs_mod);
error_dealloc:
	kfree(clp);
error_0:
	return ERR_PTR(err);
}


// Source: client.c
// Lines 148-193
