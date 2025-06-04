idmap_pipe_destroy_msg(struct rpc_pipe_msg *msg)
{
	struct idmap_legacy_upcalldata *data = container_of(msg,
			struct idmap_legacy_upcalldata,
			pipe_msg);
	struct idmap *idmap = data->idmap;

	if (msg->errno)
		nfs_idmap_abort_pipe_upcall(idmap, msg->errno);
}


// Source: nfs4idmap.c
// Lines 722-731
