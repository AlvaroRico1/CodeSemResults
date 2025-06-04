static int nfs_idmap_pipe_create(struct dentry *dir,
		struct rpc_pipe_dir_object *pdo)
{
	struct idmap *idmap = pdo->pdo_data;
	struct rpc_pipe *pipe = idmap->idmap_pipe;
	struct dentry *dentry;

	dentry = rpc_mkpipe_dentry(dir, "idmap", idmap, pipe);
	if (IS_ERR(dentry))
		return PTR_ERR(dentry);
	pipe->dentry = dentry;
	return 0;
}


// Source: nfs4idmap.c
// Lines 434-446
