static void nfs4_reset_seqids(struct nfs_server *server,
	int (*mark_reclaim)(struct nfs_client *clp, struct nfs4_state *state))
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
		sp->so_seqid.flags = 0;
		spin_lock(&sp->so_lock);
		list_for_each_entry(state, &sp->so_states, open_states) {
			if (mark_reclaim(clp, state))
				nfs4_clear_open_state(state);
		}
		spin_unlock(&sp->so_lock);
	}
	spin_unlock(&clp->cl_lock);
}


// Source: nfs4state.c
// Lines 1726-1748
