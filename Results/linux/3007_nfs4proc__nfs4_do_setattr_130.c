static int _nfs4_do_setattr(struct inode *inode,
			    struct nfs_setattrargs *arg,
			    struct nfs_setattrres *res,
			    const struct cred *cred,
			    struct nfs_open_context *ctx)
{
	struct nfs_server *server = NFS_SERVER(inode);
	struct rpc_message msg = {
		.rpc_proc	= &nfs4_procedures[NFSPROC4_CLNT_SETATTR],
		.rpc_argp	= arg,
		.rpc_resp	= res,
		.rpc_cred	= cred,
	};
	const struct cred *delegation_cred = NULL;
	unsigned long timestamp = jiffies;
	bool truncate;
	int status;

	nfs_fattr_init(res->fattr);

	/* Servers should only apply open mode checks for file size changes */
	truncate = (arg->iap->ia_valid & ATTR_SIZE) ? true : false;
	if (!truncate)
		goto zero_stateid;

	if (nfs4_copy_delegation_stateid(inode, FMODE_WRITE, &arg->stateid, &delegation_cred)) {
		/* Use that stateid */
	} else if (ctx != NULL && ctx->state) {
		struct nfs_lock_context *l_ctx;
		if (!nfs4_valid_open_stateid(ctx->state))
			return -EBADF;
		l_ctx = nfs_get_lock_context(ctx);
		if (IS_ERR(l_ctx))
			return PTR_ERR(l_ctx);
		status = nfs4_select_rw_stateid(ctx->state, FMODE_WRITE, l_ctx,
						&arg->stateid, &delegation_cred);
		nfs_put_lock_context(l_ctx);
		if (status == -EIO)
			return -EBADF;
	} else {
zero_stateid:
		nfs4_stateid_copy(&arg->stateid, &zero_stateid);
	}
	if (delegation_cred)
		msg.rpc_cred = delegation_cred;

	status = nfs4_call_sync(server->client, server, &msg, &arg->seq_args, &res->seq_res, 1);

	put_cred(delegation_cred);
	if (status == 0 && ctx != NULL)
		renew_lease(server, timestamp);
	trace_nfs4_setattr(inode, &arg->stateid, status);
	return status;
}


// Source: nfs4proc.c
// Lines 3192-3245
