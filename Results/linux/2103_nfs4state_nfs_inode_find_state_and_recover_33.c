void nfs_inode_find_state_and_recover(struct inode *inode,
		const nfs4_stateid *stateid)
{
	struct nfs_client *clp = NFS_SERVER(inode)->nfs_client;
	struct nfs_inode *nfsi = NFS_I(inode);
	struct nfs_open_context *ctx;
	struct nfs4_state *state;
	bool found = false;

	rcu_read_lock();
	list_for_each_entry_rcu(ctx, &nfsi->open_files, list) {
		state = ctx->state;
		if (state == NULL)
			continue;
		if (nfs4_stateid_match_other(&state->stateid, stateid) &&
		    nfs4_state_mark_reclaim_nograce(clp, state)) {
			found = true;
			continue;
		}
		if (nfs4_stateid_match_other(&state->open_stateid, stateid) &&
		    nfs4_state_mark_reclaim_nograce(clp, state)) {
			found = true;
			continue;
		}
		if (nfs_state_lock_state_matches_stateid(state, stateid) &&
		    nfs4_state_mark_reclaim_nograce(clp, state))
			found = true;
	}
	rcu_read_unlock();

	nfs_inode_find_delegation_state_and_recover(inode, stateid);
	if (found)
		nfs4_schedule_state_manager(clp);
}


// Source: nfs4state.c
// Lines 1446-1479
