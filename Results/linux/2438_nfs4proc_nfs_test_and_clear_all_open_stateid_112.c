static void nfs_test_and_clear_all_open_stateid(struct nfs4_state *state)
{
	struct nfs_client *clp = state->owner->so_server->nfs_client;
	bool need_recover = false;

	if (test_and_clear_bit(NFS_O_RDONLY_STATE, &state->flags) && state->n_rdonly)
		need_recover = true;
	if (test_and_clear_bit(NFS_O_WRONLY_STATE, &state->flags) && state->n_wronly)
		need_recover = true;
	if (test_and_clear_bit(NFS_O_RDWR_STATE, &state->flags) && state->n_rdwr)
		need_recover = true;
	if (need_recover)
		nfs4_state_mark_reclaim_nograce(clp, state);
}


// Source: nfs4proc.c
// Lines 1502-1515
