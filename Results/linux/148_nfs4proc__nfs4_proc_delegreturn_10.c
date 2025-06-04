static int _nfs4_proc_delegreturn(struct inode *inode, const struct cred *cred, const nfs4_stateid *stateid, int issync)
{
	struct nfs4_delegreturndata *data;
	struct nfs_server *server = NFS_SERVER(inode);
	struct rpc_task *task;
	struct rpc_message msg = {
		.rpc_proc = &nfs4_procedures[NFSPROC4_CLNT_DELEGRETURN],
		.rpc_cred = cred,
	};
	struct rpc_task_setup task_setup_data = {
		.rpc_client = server->client,
		.rpc_message = &msg,
		.callback_ops = &nfs4_delegreturn_ops,
		.flags = RPC_TASK_ASYNC,
	};
	int status = 0;

	data = kzalloc(sizeof(*data), GFP_NOFS);
	if (data == NULL)
		return -ENOMEM;
	nfs4_init_sequence(&data->args.seq_args, &data->res.seq_res, 1, 0);

	nfs4_state_protect(server->nfs_client,
			NFS_SP4_MACH_CRED_CLEANUP,
			&task_setup_data.rpc_client, &msg);

	data->args.fhandle = &data->fh;
	data->args.stateid = &data->stateid;
	data->args.bitmask = server->cache_consistency_bitmask;
	nfs_copy_fh(&data->fh, NFS_FH(inode));
	nfs4_stateid_copy(&data->stateid, stateid);
	data->res.fattr = &data->fattr;
	data->res.server = server;
	data->res.lr_ret = -NFS4ERR_NOMATCHING_LAYOUT;
	data->lr.arg.ld_private = &data->lr.ld_private;
	nfs_fattr_init(data->res.fattr);
	data->timestamp = jiffies;
	data->rpc_status = 0;
	data->lr.roc = pnfs_roc(inode, &data->lr.arg, &data->lr.res, cred);
	data->inode = nfs_igrab_and_active(inode);
	if (data->inode) {
		if (data->lr.roc) {
			data->args.lr_args = &data->lr.arg;
			data->res.lr_res = &data->lr.res;
		}
	} else if (data->lr.roc) {
		pnfs_roc_release(&data->lr.arg, &data->lr.res, 0);
		data->lr.roc = false;
	}

	task_setup_data.callback_data = data;
	msg.rpc_argp = &data->args;
	msg.rpc_resp = &data->res;
	task = rpc_run_task(&task_setup_data);
	if (IS_ERR(task))
		return PTR_ERR(task);
	if (!issync)
		goto out;
	status = rpc_wait_for_completion_task(task);
	if (status != 0)
		goto out;
	status = data->rpc_status;
out:
	rpc_put_task(task);
	return status;
}


// Source: nfs4proc.c
// Lines 6245-6310
