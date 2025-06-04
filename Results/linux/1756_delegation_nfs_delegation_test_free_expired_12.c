nfs_delegation_test_free_expired(struct inode *inode,
		nfs4_stateid *stateid,
		const struct cred *cred)
{
	struct nfs_server *server = NFS_SERVER(inode);
	const struct nfs4_minor_version_ops *ops = server->nfs_client->cl_mvops;
	int status;

	if (!cred)
		return;
	status = ops->test_and_free_expired(server, stateid, cred);
	if (status == -NFS4ERR_EXPIRED || status == -NFS4ERR_BAD_STATEID)
		nfs_remove_bad_delegation(inode, stateid);
}


// Source: delegation.c
// Lines 1050-1063
