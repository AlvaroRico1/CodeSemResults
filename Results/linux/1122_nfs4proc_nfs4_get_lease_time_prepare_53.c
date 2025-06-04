static void nfs4_get_lease_time_prepare(struct rpc_task *task,
					void *calldata)
{
	struct nfs4_get_lease_time_data *data =
			(struct nfs4_get_lease_time_data *)calldata;

	dprintk("--> %s\n", __func__);
	/* just setup sequence, do not trigger session recovery
	   since we're invoked within one */
	nfs4_setup_sequence(data->clp,
			&data->args->la_seq_args,
			&data->res->lr_seq_res,
			task);
	dprintk("<-- %s\n", __func__);
}


// Source: nfs4proc.c
// Lines 8298-8312
