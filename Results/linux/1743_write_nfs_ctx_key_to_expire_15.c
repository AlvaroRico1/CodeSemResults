bool nfs_ctx_key_to_expire(struct nfs_open_context *ctx, struct inode *inode)
{
	struct rpc_auth *auth = NFS_SERVER(inode)->client->cl_auth;
	struct rpc_cred *cred = ctx->ll_cred;
	struct auth_cred acred = {
		.cred = ctx->cred,
	};

	if (cred && !cred->cr_ops->crmatch(&acred, cred, 0)) {
		put_rpccred(cred);
		ctx->ll_cred = NULL;
		cred = NULL;
	}
	if (!cred)
		cred = auth->au_ops->lookup_cred(auth, &acred, 0);
	if (!cred || IS_ERR(cred))
		return true;
	ctx->ll_cred = cred;
	return !!(cred->cr_ops->crkey_timeout &&
		  cred->cr_ops->crkey_timeout(cred));
}


// Source: write.c
// Lines 1254-1274
