static int nfs_idmap_legacy_upcall(struct key *authkey, void *aux)
{
	struct idmap_legacy_upcalldata *data;
	struct request_key_auth *rka = get_request_key_auth(authkey);
	struct rpc_pipe_msg *msg;
	struct idmap_msg *im;
	struct idmap *idmap = (struct idmap *)aux;
	struct key *key = rka->target_key;
	int ret = -ENOKEY;

	if (!aux)
		goto out1;

	/* msg and im are freed in idmap_pipe_destroy_msg */
	ret = -ENOMEM;
	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		goto out1;

	msg = &data->pipe_msg;
	im = &data->idmap_msg;
	data->idmap = idmap;
	data->authkey = key_get(authkey);

	ret = nfs_idmap_prepare_message(key->description, idmap, im, msg);
	if (ret < 0)
		goto out2;

	ret = -EAGAIN;
	if (!nfs_idmap_prepare_pipe_upcall(idmap, data))
		goto out2;

	ret = rpc_queue_upcall(idmap->idmap_pipe, msg);
	if (ret < 0)
		nfs_idmap_abort_pipe_upcall(idmap, ret);

	return ret;
out2:
	kfree(data);
out1:
	complete_request_key(authkey, ret);
	return ret;
}


// Source: nfs4idmap.c
// Lines 581-623
