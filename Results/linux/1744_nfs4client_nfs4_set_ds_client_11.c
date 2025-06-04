struct nfs_client *nfs4_set_ds_client(struct nfs_server *mds_srv,
		const struct sockaddr *ds_addr, int ds_addrlen,
		int ds_proto, unsigned int ds_timeo, unsigned int ds_retrans,
		u32 minor_version)
{
	struct rpc_timeout ds_timeout;
	struct nfs_client *mds_clp = mds_srv->nfs_client;
	struct nfs_client_initdata cl_init = {
		.addr = ds_addr,
		.addrlen = ds_addrlen,
		.nodename = mds_clp->cl_rpcclient->cl_nodename,
		.ip_addr = mds_clp->cl_ipaddr,
		.nfs_mod = &nfs_v4,
		.proto = ds_proto,
		.minorversion = minor_version,
		.net = mds_clp->cl_net,
		.timeparms = &ds_timeout,
		.cred = mds_srv->cred,
	};
	char buf[INET6_ADDRSTRLEN + 1];

	if (rpc_ntop(ds_addr, buf, sizeof(buf)) <= 0)
		return ERR_PTR(-EINVAL);
	cl_init.hostname = buf;

	if (mds_clp->cl_nconnect > 1 && ds_proto == XPRT_TRANSPORT_TCP)
		cl_init.nconnect = mds_clp->cl_nconnect;

	if (mds_srv->flags & NFS_MOUNT_NORESVPORT)
		__set_bit(NFS_CS_NORESVPORT, &cl_init.init_flags);

	/*
	 * Set an authflavor equual to the MDS value. Use the MDS nfs_client
	 * cl_ipaddr so as to use the same EXCHANGE_ID co_ownerid as the MDS
	 * (section 13.1 RFC 5661).
	 */
	nfs_init_timeout_values(&ds_timeout, ds_proto, ds_timeo, ds_retrans);
	return nfs_get_client(&cl_init);
}


// Source: nfs4client.c
// Lines 925-963
