static void nfs4_state_end_reclaim_reboot(struct nfs_client *clp)
{
	const struct nfs4_state_recovery_ops *ops;
	const struct cred *cred;
	int err;

	if (!nfs4_state_clear_reclaim_reboot(clp))
		return;
	ops = clp->cl_mvops->reboot_recovery_ops;
	cred = nfs4_get_clid_cred(clp);
	err = nfs4_reclaim_complete(clp, ops, cred);
	put_cred(cred);
	if (err == -NFS4ERR_CONN_NOT_BOUND_TO_SESSION)
		set_bit(NFS4CLNT_RECLAIM_REBOOT, &clp->cl_state);
}


// Source: nfs4state.c
// Lines 1818-1832
