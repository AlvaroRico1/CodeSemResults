static void nfs4_open_prepare(struct rpc_task *task, void *calldata)
{
	struct nfs4_opendata *data = calldata;
	struct nfs4_state_owner *sp = data->owner;
	struct nfs_client *clp = sp->so_server->nfs_client;
	enum open_claim_type4 claim = data->o_arg.claim;

	if (nfs_wait_on_sequence(data->o_arg.seqid, task) != 0)
		goto out_wait;
	/*
	 * Check if we still need to send an OPEN call, or if we can use
	 * a delegation instead.
	 */
	if (data->state != NULL) {
		struct nfs_delegation *delegation;

		if (can_open_cached(data->state, data->o_arg.fmode,
					data->o_arg.open_flags, claim))
			goto out_no_action;
		rcu_read_lock();
		delegation = rcu_dereference(NFS_I(data->state->inode)->delegation);
		if (can_open_delegated(delegation, data->o_arg.fmode, claim))
			goto unlock_no_action;
		rcu_read_unlock();
	}
	/* Update client id. */
	data->o_arg.clientid = clp->cl_clientid;
	switch (claim) {
	default:
		break;
	case NFS4_OPEN_CLAIM_PREVIOUS:
	case NFS4_OPEN_CLAIM_DELEG_CUR_FH:
	case NFS4_OPEN_CLAIM_DELEG_PREV_FH:
		data->o_arg.open_bitmap = &nfs4_open_noattr_bitmap[0];
		/* Fall through */
	case NFS4_OPEN_CLAIM_FH:
		task->tk_msg.rpc_proc = &nfs4_procedures[NFSPROC4_CLNT_OPEN_NOATTR];
	}
	data->timestamp = jiffies;
	if (nfs4_setup_sequence(data->o_arg.server->nfs_client,
				&data->o_arg.seq_args,
				&data->o_res.seq_res,
				task) != 0)
		nfs_release_seqid(data->o_arg.seqid);

	/* Set the create mode (note dependency on the session type) */
	data->o_arg.createmode = NFS4_CREATE_UNCHECKED;
	if (data->o_arg.open_flags & O_EXCL) {
		data->o_arg.createmode = NFS4_CREATE_EXCLUSIVE;
		if (nfs4_has_persistent_session(clp))
			data->o_arg.createmode = NFS4_CREATE_GUARDED;
		else if (clp->cl_mvops->minor_version > 0)
			data->o_arg.createmode = NFS4_CREATE_EXCLUSIVE4_1;
	}
	return;
unlock_no_action:
	trace_nfs4_cached_open(data->state);
	rcu_read_unlock();
out_no_action:
	task->tk_action = NULL;
out_wait:
	nfs4_sequence_done(task, &data->o_res.seq_res);
}


// Source: nfs4proc.c
// Lines 2349-2411
