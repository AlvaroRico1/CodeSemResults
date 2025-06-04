static struct nfs_client *nfs_match_client(const struct nfs_client_initdata *data)
{
	struct nfs_client *clp;
	const struct sockaddr *sap = data->addr;
	struct nfs_net *nn = net_generic(data->net, nfs_net_id);
	int error;

again:
	list_for_each_entry(clp, &nn->nfs_client_list, cl_share_link) {
	        const struct sockaddr *clap = (struct sockaddr *)&clp->cl_addr;
		/* Don't match clients that failed to initialise properly */
		if (clp->cl_cons_state < 0)
			continue;

		/* If a client is still initializing then we need to wait */
		if (clp->cl_cons_state > NFS_CS_READY) {
			refcount_inc(&clp->cl_count);
			spin_unlock(&nn->nfs_client_lock);
			error = nfs_wait_client_init_complete(clp);
			nfs_put_client(clp);
			spin_lock(&nn->nfs_client_lock);
			if (error < 0)
				return ERR_PTR(error);
			goto again;
		}

		/* Different NFS versions cannot share the same nfs_client */
		if (clp->rpc_ops != data->nfs_mod->rpc_ops)
			continue;

		if (clp->cl_proto != data->proto)
			continue;
		/* Match nfsv4 minorversion */
		if (clp->cl_minorversion != data->minorversion)
			continue;
		/* Match the full socket address */
		if (!rpc_cmp_addr_port(sap, clap))
			/* Match all xprt_switch full socket addresses */
			if (IS_ERR(clp->cl_rpcclient) ||
                            !rpc_clnt_xprt_switch_has_addr(clp->cl_rpcclient,
							   sap))
				continue;

		refcount_inc(&clp->cl_count);
		return clp;
	}
	return NULL;
}


// Source: client.c
// Lines 280-327
