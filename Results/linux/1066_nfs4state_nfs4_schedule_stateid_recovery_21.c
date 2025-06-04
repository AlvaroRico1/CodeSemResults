int nfs4_schedule_stateid_recovery(const struct nfs_server *server, struct nfs4_state *state)
{
	struct nfs_client *clp = server->nfs_client;

	if (!nfs4_state_mark_reclaim_nograce(clp, state))
		return -EBADF;
	nfs_inode_find_delegation_state_and_recover(state->inode,
			&state->stateid);
	dprintk("%s: scheduling stateid recovery for server %s\n", __func__,
			clp->cl_hostname);
	nfs4_schedule_state_manager(clp);
	return 0;
}


// Source: nfs4state.c
// Lines 1402-1414
