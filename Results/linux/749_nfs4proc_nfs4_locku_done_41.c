static void nfs4_locku_done(struct rpc_task *task, void *data)
{
	struct nfs4_unlockdata *calldata = data;
	struct nfs4_exception exception = {
		.inode = calldata->lsp->ls_state->inode,
		.stateid = &calldata->arg.stateid,
	};

	if (!nfs4_sequence_done(task, &calldata->res.seq_res))
		return;
	switch (task->tk_status) {
		case 0:
			renew_lease(calldata->server, calldata->timestamp);
			locks_lock_inode_wait(calldata->lsp->ls_state->inode, &calldata->fl);
			if (nfs4_update_lock_stateid(calldata->lsp,
					&calldata->res.stateid))
				break;
			/* Fall through */
		case -NFS4ERR_ADMIN_REVOKED:
		case -NFS4ERR_EXPIRED:
			nfs4_free_revoked_stateid(calldata->server,
					&calldata->arg.stateid,
					task->tk_msg.rpc_cred);
			/* Fall through */
		case -NFS4ERR_BAD_STATEID:
		case -NFS4ERR_OLD_STATEID:
		case -NFS4ERR_STALE_STATEID:
			if (!nfs4_stateid_match(&calldata->arg.stateid,
						&calldata->lsp->ls_stateid))
				rpc_restart_call_prepare(task);
			break;
		default:
			task->tk_status = nfs4_async_handle_exception(task,
					calldata->server, task->tk_status,
					&exception);
			if (exception.retry)
				rpc_restart_call_prepare(task);
	}
	nfs_release_seqid(calldata->arg.seqid);
}


// Source: nfs4proc.c
// Lines 6435-6474
