static void nfs4_release_lockowner_prepare(struct rpc_task *task, void *calldata)
{
	struct nfs_release_lockowner_data *data = calldata;
	struct nfs_server *server = data->server;
	nfs4_setup_sequence(server->nfs_client, &data->args.seq_args,
			   &data->res.seq_res, task);
	data->args.lock_owner.clientid = server->nfs_client->cl_clientid;
	data->timestamp = jiffies;
}


// Source: nfs4proc.c
// Lines 7137-7145
