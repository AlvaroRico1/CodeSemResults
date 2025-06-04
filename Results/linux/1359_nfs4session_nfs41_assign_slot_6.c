static bool nfs41_assign_slot(struct rpc_task *task, void *pslot)
{
	struct nfs4_sequence_args *args = task->tk_msg.rpc_argp;
	struct nfs4_sequence_res *res = task->tk_msg.rpc_resp;
	struct nfs4_slot *slot = pslot;
	struct nfs4_slot_table *tbl = slot->table;

	if (nfs4_slot_tbl_draining(tbl) && !args->sa_privileged)
		return false;
	slot->generation = tbl->generation;
	args->sa_slot = slot;
	res->sr_timestamp = jiffies;
	res->sr_slot = slot;
	res->sr_status_flags = 0;
	res->sr_status = 1;
	return true;
}


// Source: nfs4session.c
// Lines 357-373
