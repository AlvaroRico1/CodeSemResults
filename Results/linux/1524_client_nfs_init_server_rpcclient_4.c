int nfs_init_server_rpcclient(struct nfs_server *server,
		const struct rpc_timeout *timeo,
		rpc_authflavor_t pseudoflavour)
{
	struct nfs_client *clp = server->nfs_client;

	server->client = rpc_clone_client_set_auth(clp->cl_rpcclient,
							pseudoflavour);
	if (IS_ERR(server->client)) {
		dprintk("%s: couldn't create rpc_client!\n", __func__);
		return PTR_ERR(server->client);
	}

	memcpy(&server->client->cl_timeout_default,
			timeo,
			sizeof(server->client->cl_timeout_default));
	server->client->cl_timeout = &server->client->cl_timeout_default;
	server->client->cl_softrtry = 0;
	if (server->flags & NFS_MOUNT_SOFTERR)
		server->client->cl_softerr = 1;
	if (server->flags & NFS_MOUNT_SOFT)
		server->client->cl_softrtry = 1;

	return 0;
}


// Source: client.c
// Lines 589-613
