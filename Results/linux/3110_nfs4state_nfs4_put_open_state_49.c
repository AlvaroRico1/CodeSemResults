void nfs4_put_open_state(struct nfs4_state *state)
{
	struct inode *inode = state->inode;
	struct nfs4_state_owner *owner = state->owner;

	if (!refcount_dec_and_lock(&state->count, &owner->so_lock))
		return;
	spin_lock(&inode->i_lock);
	list_del_rcu(&state->inode_states);
	list_del(&state->open_states);
	spin_unlock(&inode->i_lock);
	spin_unlock(&owner->so_lock);
	iput(inode);
	nfs4_free_open_state(state);
	nfs4_put_state_owner(owner);
}


// Source: nfs4state.c
// Lines 756-771
