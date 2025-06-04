idmap_release_pipe(struct inode *inode)
{
	struct rpc_inode *rpci = RPC_I(inode);
	struct idmap *idmap = (struct idmap *)rpci->private;

	nfs_idmap_abort_pipe_upcall(idmap, -EPIPE);
}


// Source: nfs4idmap.c
// Lines 734-740
