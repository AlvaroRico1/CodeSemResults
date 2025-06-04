static void nfs4_end_drain_session(struct nfs_client *clp)
{
	struct nfs4_session *ses = clp->cl_session;

	if (clp->cl_slot_tbl) {
		nfs4_end_drain_slot_table(clp->cl_slot_tbl);
		return;
	}

	if (ses != NULL) {
		nfs4_end_drain_slot_table(&ses->bc_slot_table);
		nfs4_end_drain_slot_table(&ses->fc_slot_table);
	}
}


// Source: nfs4state.c
// Lines 264-277
