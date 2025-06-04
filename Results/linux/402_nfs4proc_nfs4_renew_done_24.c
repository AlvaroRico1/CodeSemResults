static void nfs4_renew_done(struct rpc_task *task, void *calldata)
{
	struct nfs4_renewdata *data = calldata;
	struct nfs_client *clp = data->client;
	unsigned long timestamp = data->timestamp;

	trace_nfs4_renew_async(clp, task->tk_status);
	switch (task->tk_status) {
	case 0:
		break;
	case -NFS4ERR_LEASE_MOVED:
		nfs4_schedule_lease_moved_recovery(clp);
		break;
	default:
		/* Unless we're shutting down, schedule state recovery! */
		if (test_bit(NFS_CS_RENEWD, &clp->cl_res_state) == 0)
			return;
		if (task->tk_status != NFS4ERR_CB_PATH_DOWN) {
			nfs4_schedule_lease_recovery(clp);
			return;
		}
		nfs4_schedule_path_down_recovery(clp);
	}
	do_renew_lease(clp, timestamp);
}


// Source: nfs4proc.c
// Lines 5334-5358
