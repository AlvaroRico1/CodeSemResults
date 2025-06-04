static void nfs_idmap_pipe_destroy(struct dentry *dir,
		struct rpc_pipe_dir_object *pdo)
{
	struct idmap *idmap = pdo->pdo_data;
	struct rpc_pipe *pipe = idmap->idmap_pipe;

	if (pipe->dentry) {
		rpc_unlink(pipe->dentry);
		pipe->dentry = NULL;
	}
}


// Source: nfs4idmap.c
// Lines 422-432
