nfs_idmap_delete(struct nfs_client *clp)
{
	struct idmap *idmap = clp->cl_idmap;

	if (!idmap)
		return;
	clp->cl_idmap = NULL;
	rpc_remove_pipe_dir_object(clp->cl_net,
			&clp->cl_rpcclient->cl_pipedir_objects,
			&idmap->idmap_pdo);
	rpc_destroy_pipe_data(idmap->idmap_pipe);
	put_cred(idmap->cred);
	kfree(idmap);
}


// Source: nfs4idmap.c
// Lines 495-508
