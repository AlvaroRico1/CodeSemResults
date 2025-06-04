static void __put_nfs_open_context(struct nfs_open_context *ctx, int is_sync)
{
	struct inode *inode = d_inode(ctx->dentry);
	struct super_block *sb = ctx->dentry->d_sb;

	if (!refcount_dec_and_test(&ctx->lock_context.count))
		return;
	if (!list_empty(&ctx->list)) {
		spin_lock(&inode->i_lock);
		list_del_rcu(&ctx->list);
		spin_unlock(&inode->i_lock);
	}
	if (inode != NULL)
		NFS_PROTO(inode)->close_context(ctx, is_sync);
	put_cred(ctx->cred);
	dput(ctx->dentry);
	nfs_sb_deactive(sb);
	put_rpccred(ctx->ll_cred);
	kfree(ctx->mdsthreshold);
	kfree_rcu(ctx, rcu_head);
}


// Source: inode.c
// Lines 994-1014
