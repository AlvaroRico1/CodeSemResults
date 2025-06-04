static struct nfs4_lock_state *nfs4_get_lock_state(struct nfs4_state *state, fl_owner_t owner)
{
	struct nfs4_lock_state *lsp, *new = NULL;
	
	for(;;) {
		spin_lock(&state->state_lock);
		lsp = __nfs4_find_lock_state(state, owner, NULL);
		if (lsp != NULL)
			break;
		if (new != NULL) {
			list_add(&new->ls_locks, &state->lock_states);
			set_bit(LK_STATE_IN_USE, &state->flags);
			lsp = new;
			new = NULL;
			break;
		}
		spin_unlock(&state->state_lock);
		new = nfs4_alloc_lock_state(state, owner);
		if (new == NULL)
			return NULL;
	}
	spin_unlock(&state->state_lock);
	if (new != NULL)
		nfs4_free_lock_state(state->owner->so_server, new);
	return lsp;
}


// Source: nfs4state.c
// Lines 901-926
