struct nfs_client *nfs4_init_client(struct nfs_client *clp,
				    const struct nfs_client_initdata *cl_init)
{
	char buf[INET6_ADDRSTRLEN + 1];
	const char *ip_addr = cl_init->ip_addr;
	struct nfs_client *old;
	int error;

	if (clp->cl_cons_state == NFS_CS_READY)
		/* the client is initialised already */
		return clp;

	/* Check NFS protocol revision and initialize RPC op vector */
	clp->rpc_ops = &nfs_v4_clientops;

	if (clp->cl_minorversion != 0)
		__set_bit(NFS_CS_INFINITE_SLOTS, &clp->cl_flags);
	__set_bit(NFS_CS_DISCRTRY, &clp->cl_flags);
	__set_bit(NFS_CS_NO_RETRANS_TIMEOUT, &clp->cl_flags);

	error = nfs_create_rpc_client(clp, cl_init, RPC_AUTH_GSS_KRB5I);
	if (error == -EINVAL)
		error = nfs_create_rpc_client(clp, cl_init, RPC_AUTH_UNIX);
	if (error < 0)
		goto error;

	/* If no clientaddr= option was specified, find a usable cb address */
	if (ip_addr == NULL) {
		struct sockaddr_storage cb_addr;
		struct sockaddr *sap = (struct sockaddr *)&cb_addr;

		error = rpc_localaddr(clp->cl_rpcclient, sap, sizeof(cb_addr));
		if (error < 0)
			goto error;
		error = rpc_ntop(sap, buf, sizeof(buf));
		if (error < 0)
			goto error;
		ip_addr = (const char *)buf;
	}


// Source: nfs4client.c
// Lines 373-411
