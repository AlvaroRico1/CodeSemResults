int nfs4_do_close(struct nfs4_state *state, gfp_t gfp_mask, int wait)
{
	struct nfs_server *server = NFS_SERVER(state->inode);
	struct nfs_seqid *(*alloc_seqid)(struct nfs_seqid_counter *, gfp_t);
	struct nfs4_closedata *calldata;
	struct nfs4_state_owner *sp = state->owner;
	struct rpc_task *task;
	struct rpc_message msg = {
		.rpc_proc = &nfs4_procedures[NFSPROC4_CLNT_CLOSE],
		.rpc_cred = state->owner->so_cred,
	};
	struct rpc_task_setup task_setup_data = {
		.rpc_client = server->client,
		.rpc_message = &msg,
		.callback_ops = &nfs4_close_ops,
		.workqueue = nfsiod_workqueue,
		.flags = RPC_TASK_ASYNC,
	};
	int status = -ENOMEM;

	nfs4_state_protect(server->nfs_client, NFS_SP4_MACH_CRED_CLEANUP,
		&task_setup_data.rpc_client, &msg);

	calldata = kzalloc(sizeof(*calldata), gfp_mask);
	if (calldata == NULL)
		goto out;
	nfs4_init_sequence(&calldata->arg.seq_args, &calldata->res.seq_res, 1, 0);
	calldata->inode = state->inode;
	calldata->state = state;
	calldata->arg.fh = NFS_FH(state->inode);
	if (!nfs4_copy_open_stateid(&calldata->arg.stateid, state))
		goto out_free_calldata;
	/* Serialization for the sequence id */
	alloc_seqid = server->nfs_client->cl_mvops->alloc_seqid;
	calldata->arg.seqid = alloc_seqid(&state->owner->so_seqid, gfp_mask);
	if (IS_ERR(calldata->arg.seqid))
		goto out_free_calldata;
	nfs_fattr_init(&calldata->fattr);
	calldata->arg.fmode = 0;
	calldata->lr.arg.ld_private = &calldata->lr.ld_private;
	calldata->res.fattr = &calldata->fattr;
	calldata->res.seqid = calldata->arg.seqid;
	calldata->res.server = server;
	calldata->res.lr_ret = -NFS4ERR_NOMATCHING_LAYOUT;
	calldata->lr.roc = pnfs_roc(state->inode,
			&calldata->lr.arg, &calldata->lr.res, msg.rpc_cred);
	if (calldata->lr.roc) {
		calldata->arg.lr_args = &calldata->lr.arg;
		calldata->res.lr_res = &calldata->lr.res;
	}
	nfs_sb_active(calldata->inode->i_sb);

	msg.rpc_argp = &calldata->arg;
	msg.rpc_resp = &calldata->res;
	task_setup_data.callback_data = calldata;
	task = rpc_run_task(&task_setup_data);
	if (IS_ERR(task))
		return PTR_ERR(task);
	status = 0;
	if (wait)
		status = rpc_wait_for_completion_task(task);
	rpc_put_task(task);
	return status;
out_free_calldata:
	kfree(calldata);
out:
	nfs4_put_open_state(state);
	nfs4_put_state_owner(sp);
	return status;
}


// Source: nfs4proc.c
// Lines 3546-3615
