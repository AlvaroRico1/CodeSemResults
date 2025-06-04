static void nfs40_call_sync_done(struct rpc_task *task, void *calldata)
{
	struct nfs4_call_sync_data *data = calldata;
	nfs4_sequence_done(task, data->seq_res);
}


// Source: nfs4proc.c
// Lines 1065-1069
