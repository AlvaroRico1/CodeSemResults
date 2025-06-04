static void nfs4_open_confirm_prepare(struct rpc_task *task, void *calldata)
{
	struct nfs4_opendata *data = calldata;

	nfs4_setup_sequence(data->o_arg.server->nfs_client,
			   &data->c_arg.seq_args, &data->c_res.seq_res, task);
}


// Source: nfs4proc.c
// Lines 2260-2266
