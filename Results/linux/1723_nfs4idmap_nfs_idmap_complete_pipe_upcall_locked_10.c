nfs_idmap_complete_pipe_upcall_locked(struct idmap *idmap, int ret)
{
	struct key *authkey = idmap->idmap_upcall_data->authkey;

	kfree(idmap->idmap_upcall_data);
	idmap->idmap_upcall_data = NULL;
	complete_request_key(authkey, ret);
	key_put(authkey);
}


// Source: nfs4idmap.c
// Lines 564-572
