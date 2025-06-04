static void nfs4_test_and_free_stateid(struct nfs_server *server,
		nfs4_stateid *stateid,
		const struct cred *cred)
{
	const struct nfs4_minor_version_ops *ops = server->nfs_client->cl_mvops;

	ops->test_and_free_expired(server, stateid, cred);
}


// Source: nfs4proc.c
// Lines 362-369
