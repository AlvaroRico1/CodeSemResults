static bool nfs4_update_lock_stateid(struct nfs4_lock_state *lsp,
		const nfs4_stateid *stateid)
{
	struct nfs4_state *state = lsp->ls_state;
	bool ret = false;

	spin_lock(&state->state_lock);
	if (!nfs4_stateid_match_other(stateid, &lsp->ls_stateid))
		goto out_noupdate;
	if (!nfs4_stateid_is_newer(stateid, &lsp->ls_stateid))
		goto out_noupdate;
	nfs4_stateid_copy(&lsp->ls_stateid, stateid);
	ret = true;
out_noupdate:
	spin_unlock(&state->state_lock);
	return ret;
}


// Source: nfs4proc.c
// Lines 1772-1788
