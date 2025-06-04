__nfs4_find_lock_state(struct nfs4_state *state,
		       fl_owner_t fl_owner, fl_owner_t fl_owner2)
{
	struct nfs4_lock_state *pos, *ret = NULL;
	list_for_each_entry(pos, &state->lock_states, ls_locks) {
		if (pos->ls_owner == fl_owner) {
			ret = pos;
			break;
		}
		if (pos->ls_owner == fl_owner2)
			ret = pos;
	}
	if (ret)
		refcount_inc(&ret->ls_count);
	return ret;
}


// Source: nfs4state.c
// Lines 845-860
