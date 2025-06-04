static void nfs4_state_mark_open_context_bad(struct nfs4_state *state, int err)
{
	struct inode *inode = state->inode;
	struct nfs_inode *nfsi = NFS_I(inode);
	struct nfs_open_context *ctx;

	rcu_read_lock();
	list_for_each_entry_rcu(ctx, &nfsi->open_files, list) {
		if (ctx->state != state)
			continue;
		set_bit(NFS_CONTEXT_BAD, &ctx->flags);
		pr_warn("NFSv4: state recovery failed for open file %pd2, "
				"error = %d\n", ctx->dentry, err);
	}
	rcu_read_unlock();
}


// Source: nfs4state.c
// Lines 1481-1496
