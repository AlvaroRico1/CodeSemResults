static void nfs4_release_lockowner_done(struct rpc_task *task, void *calldata)
{
	struct nfs_release_lockowner_data *data = calldata;
	struct nfs_server *server = data->server;

	nfs40_sequence_done(task, &data->res.seq_res);

	switch (task->tk_status) {
	case 0:
		renew_lease(server, data->timestamp);
		break;
	case -NFS4ERR_STALE_CLIENTID:
	case -NFS4ERR_EXPIRED:
		nfs4_schedule_lease_recovery(server->nfs_client);
		break;
	case -NFS4ERR_LEASE_MOVED:
	case -NFS4ERR_DELAY:
		if (nfs4_async_handle_error(task, server,
					    NULL, NULL) == -EAGAIN)
			rpc_restart_call_prepare(task);
	}
}


// Source: nfs4proc.c
// Lines 7147-7168
