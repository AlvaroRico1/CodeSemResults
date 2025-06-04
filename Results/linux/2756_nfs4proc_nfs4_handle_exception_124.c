int nfs4_handle_exception(struct nfs_server *server, int errorcode, struct nfs4_exception *exception)
{
	struct nfs_client *clp = server->nfs_client;
	int ret;

	ret = nfs4_do_handle_exception(server, errorcode, exception);
	if (exception->delay) {
		ret = nfs4_delay(&exception->timeout,
				exception->interruptible);
		goto out_retry;
	}
	if (exception->recovering) {
		ret = nfs4_wait_clnt_recover(clp);
		if (test_bit(NFS_MIG_FAILED, &server->mig_status))
			return -EIO;
		goto out_retry;
	}
	return ret;
out_retry:
	if (ret == 0)
		exception->retry = 1;
	return ret;
}


// Source: nfs4proc.c
// Lines 574-596
