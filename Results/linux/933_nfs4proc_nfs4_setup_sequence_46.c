int nfs4_setup_sequence(struct nfs_client *client,
			struct nfs4_sequence_args *args,
			struct nfs4_sequence_res *res,
			struct rpc_task *task)
{
	struct nfs4_session *session = nfs4_get_session(client);
	struct nfs4_slot_table *tbl  = client->cl_slot_tbl;
	struct nfs4_slot *slot;

	/* slot already allocated? */
	if (res->sr_slot != NULL)
		goto out_start;

	if (session)
		tbl = &session->fc_slot_table;

	spin_lock(&tbl->slot_tbl_lock);
	/* The state manager will wait until the slot table is empty */
	if (nfs4_slot_tbl_draining(tbl) && !args->sa_privileged)
		goto out_sleep;

	slot = nfs4_alloc_slot(tbl);
	if (IS_ERR(slot)) {
		if (slot == ERR_PTR(-ENOMEM))
			goto out_sleep_timeout;
		goto out_sleep;
	}
	spin_unlock(&tbl->slot_tbl_lock);

	nfs4_sequence_attach_slot(args, res, slot);

	trace_nfs4_setup_sequence(session, args);
out_start:
	nfs41_sequence_res_init(res);
	rpc_call_start(task);
	return 0;
out_sleep_timeout:
	/* Try again in 1/4 second */
	if (args->sa_privileged)
		rpc_sleep_on_priority_timeout(&tbl->slot_tbl_waitq, task,
				jiffies + (HZ >> 2), RPC_PRIORITY_PRIVILEGED);
	else
		rpc_sleep_on_timeout(&tbl->slot_tbl_waitq, task,
				NULL, jiffies + (HZ >> 2));
	spin_unlock(&tbl->slot_tbl_lock);
	return -EAGAIN;
out_sleep:
	if (args->sa_privileged)
		rpc_sleep_on_priority(&tbl->slot_tbl_waitq, task,
				RPC_PRIORITY_PRIVILEGED);
	else
		rpc_sleep_on(&tbl->slot_tbl_waitq, task, NULL);
	spin_unlock(&tbl->slot_tbl_lock);
	return -EAGAIN;
}


// Source: nfs4proc.c
// Lines 1001-1055
