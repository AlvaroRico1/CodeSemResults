static int _nfs4_open_and_get_state(struct nfs4_opendata *opendata,
		int flags, struct nfs_open_context *ctx)
{
	struct nfs4_state_owner *sp = opendata->owner;
	struct nfs_server *server = sp->so_server;
	struct dentry *dentry;
	struct nfs4_state *state;
	fmode_t acc_mode = _nfs4_ctx_to_accessmode(ctx);
	unsigned int seq;
	int ret;

	seq = raw_seqcount_begin(&sp->so_reclaim_seqcount);

	ret = _nfs4_proc_open(opendata, ctx);
	if (ret != 0)
		goto out;

	state = _nfs4_opendata_to_nfs4_state(opendata);
	ret = PTR_ERR(state);
	if (IS_ERR(state))
		goto out;
	ctx->state = state;
	if (server->caps & NFS_CAP_POSIX_LOCK)
		set_bit(NFS_STATE_POSIX_LOCKS, &state->flags);
	if (opendata->o_res.rflags & NFS4_OPEN_RESULT_MAY_NOTIFY_LOCK)
		set_bit(NFS_STATE_MAY_NOTIFY_LOCK, &state->flags);

	dentry = opendata->dentry;
	if (d_really_is_negative(dentry)) {
		struct dentry *alias;
		d_drop(dentry);
		alias = d_exact_alias(dentry, state->inode);
		if (!alias)
			alias = d_splice_alias(igrab(state->inode), dentry);
		/* d_splice_alias() can't fail here - it's a non-directory */
		if (alias) {
			dput(ctx->dentry);
			ctx->dentry = dentry = alias;
		}
		nfs_set_verifier(dentry,
				nfs_save_change_attribute(d_inode(opendata->dir)));
	}


// Source: nfs4proc.c
// Lines 2946-2987
