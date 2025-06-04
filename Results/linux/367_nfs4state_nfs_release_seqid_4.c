void nfs_release_seqid(struct nfs_seqid *seqid)
{
	struct nfs_seqid_counter *sequence;

	if (seqid == NULL || list_empty(&seqid->list))
		return;
	sequence = seqid->sequence;
	spin_lock(&sequence->lock);
	list_del_init(&seqid->list);
	if (!list_empty(&sequence->list)) {
		struct nfs_seqid *next;

		next = list_first_entry(&sequence->list,
				struct nfs_seqid, list);
		rpc_wake_up_queued_task(&sequence->wait, next->task);
	}
	spin_unlock(&sequence->lock);
}


// Source: nfs4state.c
// Lines 1102-1119
