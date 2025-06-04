_nfs4_opendata_reclaim_to_nfs4_state(struct nfs4_opendata *data)
{
	struct inode *inode = data->state->inode;
	struct nfs4_state *state = data->state;
	int ret;

	if (!data->rpc_done) {
		if (data->rpc_status)
			return ERR_PTR(data->rpc_status);
		/* cached opens have already been processed */
		goto update;
	}

	ret = nfs_refresh_inode(inode, &data->f_attr);
	if (ret)
		return ERR_PTR(ret);

	if (data->o_res.delegation_type != 0)
		nfs4_opendata_check_deleg(data, state);
update:
	if (!update_open_stateid(state, &data->o_res.stateid,
				NULL, data->o_arg.fmode))
		return ERR_PTR(-EAGAIN);
	refcount_inc(&state->count);

	return state;
}


// Source: nfs4proc.c
// Lines 1898-1924
