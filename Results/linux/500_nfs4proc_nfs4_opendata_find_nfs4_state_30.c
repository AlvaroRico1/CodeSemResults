nfs4_opendata_find_nfs4_state(struct nfs4_opendata *data)
{
	struct nfs4_state *state;
	struct inode *inode;

	inode = nfs4_opendata_get_inode(data);
	if (IS_ERR(inode))
		return ERR_CAST(inode);
	if (data->state != NULL && data->state->inode == inode) {
		state = data->state;
		refcount_inc(&state->count);
	} else
		state = nfs4_get_open_state(inode, data->owner);
	iput(inode);
	if (state == NULL)
		state = ERR_PTR(-ENOMEM);
	return state;
}


// Source: nfs4proc.c
// Lines 1949-1966
