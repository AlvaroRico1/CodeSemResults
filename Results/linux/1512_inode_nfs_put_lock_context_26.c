void nfs_put_lock_context(struct nfs_lock_context *l_ctx)
{
	struct nfs_open_context *ctx = l_ctx->open_context;
	struct inode *inode = d_inode(ctx->dentry);

	if (!refcount_dec_and_lock(&l_ctx->count, &inode->i_lock))
		return;
	list_del_rcu(&l_ctx->list);
	spin_unlock(&inode->i_lock);
	put_nfs_open_context(ctx);
	kfree_rcu(l_ctx, rcu_head);
}


// Source: inode.c
// Lines 907-918
