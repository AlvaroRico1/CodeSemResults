static void renew_lease(const struct nfs_server *server, unsigned long timestamp)
{
	struct nfs_client *clp = server->nfs_client;

	if (!nfs4_has_session(clp))
		do_renew_lease(clp, timestamp);
}


// Source: nfs4proc.c
// Lines 672-678
