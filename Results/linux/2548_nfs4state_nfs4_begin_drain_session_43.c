static int nfs4_begin_drain_session(struct nfs_client *clp)
{
	struct nfs4_session *ses = clp->cl_session;
	int ret;

	if (clp->cl_slot_tbl)
		return nfs4_drain_slot_tbl(clp->cl_slot_tbl);

	/* back channel */
	ret = nfs4_drain_slot_tbl(&ses->bc_slot_table);
	if (ret)
		return ret;
	/* fore channel */
	return nfs4_drain_slot_tbl(&ses->fc_slot_table);
}


// Source: nfs4state.c
// Lines 292-306
