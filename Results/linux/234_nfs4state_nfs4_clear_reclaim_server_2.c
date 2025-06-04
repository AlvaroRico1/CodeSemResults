static void nfs4_clear_reclaim_server(struct nfs_server *server)
{
	struct nfs_client *clp = server->nfs_client;
	struct nfs4_state_owner *sp;
	struct rb_node *pos;
	struct nfs4_state *state;

	spin_lock(&clp->cl_lock);
	for (pos = rb_first(&server->state_owners);
	     pos != NULL;
	     pos = rb_next(pos)) {
		sp = rb_entry(pos, struct nfs4_state_owner, so_server_node);
		spin_lock(&sp->so_lock);
		list_for_each_entry(state, &sp->so_states, open_states) {
			if (!test_and_clear_bit(NFS_STATE_RECLAIM_REBOOT,
						&state->flags))
				continue;
			nfs4_state_mark_reclaim_nograce(clp, state);
		}
		spin_unlock(&sp->so_lock);
	}
	spin_unlock(&clp->cl_lock);
}


// Source: nfs4state.c
// Lines 1778-1800
