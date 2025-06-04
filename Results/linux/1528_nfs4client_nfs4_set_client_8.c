static int nfs4_set_client(struct nfs_server *server,
		const char *hostname,
		const struct sockaddr *addr,
		const size_t addrlen,
		const char *ip_addr,
		int proto, const struct rpc_timeout *timeparms,
		u32 minorversion, unsigned int nconnect,
		struct net *net)
{
	struct nfs_client_initdata cl_init = {
		.hostname = hostname,
		.addr = addr,
		.addrlen = addrlen,
		.ip_addr = ip_addr,
		.nfs_mod = &nfs_v4,
		.proto = proto,
		.minorversion = minorversion,
		.net = net,
		.timeparms = timeparms,
		.cred = server->cred,
	};
	struct nfs_client *clp;

	if (minorversion > 0 && proto == XPRT_TRANSPORT_TCP)
		cl_init.nconnect = nconnect;
	if (server->flags & NFS_MOUNT_NORESVPORT)
		set_bit(NFS_CS_NORESVPORT, &cl_init.init_flags);
	if (server->options & NFS_OPTION_MIGRATION)
		set_bit(NFS_CS_MIGRATION, &cl_init.init_flags);
	if (test_bit(NFS_MIG_TSM_POSSIBLE, &server->mig_status))
		set_bit(NFS_CS_TSM_POSSIBLE, &cl_init.init_flags);
	server->port = rpc_get_port(addr);

	/* Allocate or find a client reference we can use */
	clp = nfs_get_client(&cl_init);
	if (IS_ERR(clp))
		return PTR_ERR(clp);

	if (server->nfs_client == clp) {
		nfs_put_client(clp);
		return -ELOOP;
	}

	/*
	 * Query for the lease time on clientid setup or renewal
	 *
	 * Note that this will be set on nfs_clients that were created
	 * only for the DS role and did not set this bit, but now will
	 * serve a dual role.
	 */
	set_bit(NFS_CS_CHECK_LEASE_TIME, &clp->cl_res_state);

	server->nfs_client = clp;
	return 0;
}


// Source: nfs4client.c
// Lines 859-913
