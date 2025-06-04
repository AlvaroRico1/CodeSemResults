int nfs4_proc_get_locations(struct inode *inode,
			    struct nfs4_fs_locations *locations,
			    struct page *page, const struct cred *cred)
{
	struct nfs_server *server = NFS_SERVER(inode);
	struct nfs_client *clp = server->nfs_client;
	const struct nfs4_mig_recovery_ops *ops =
					clp->cl_mvops->mig_recovery_ops;
	struct nfs4_exception exception = {
		.interruptible = true,
	};
	int status;

	dprintk("%s: FSID %llx:%llx on \"%s\"\n", __func__,
		(unsigned long long)server->fsid.major,
		(unsigned long long)server->fsid.minor,
		clp->cl_hostname);
	nfs_display_fhandle(NFS_FH(inode), __func__);

	do {
		status = ops->get_locations(inode, locations, page, cred);
		if (status != -NFS4ERR_DELAY)
			break;
		nfs4_handle_exception(server, status, &exception);
	} while (exception.retry);
	return status;
}


// Source: nfs4proc.c
// Lines 7484-7510
