static int nfs_set_super(struct super_block *s, void *data)
{
	struct nfs_sb_mountdata *sb_mntdata = data;
	struct nfs_server *server = sb_mntdata->server;
	int ret;

	s->s_flags = sb_mntdata->mntflags;
	s->s_fs_info = server;
	s->s_d_op = server->nfs_client->rpc_ops->dentry_ops;
	ret = set_anon_super(s, server);
	if (ret == 0)
		server->s_dev = s->s_dev;
	return ret;
}


// Source: super.c
// Lines 2454-2467
