static int nfs4_establish_lease(struct nfs_client *clp)
{
	const struct cred *cred;
	const struct nfs4_state_recovery_ops *ops =
		clp->cl_mvops->reboot_recovery_ops;
	int status;

	status = nfs4_begin_drain_session(clp);
	if (status != 0)
		return status;
	cred = nfs4_get_clid_cred(clp);
	if (cred == NULL)
		return -ENOENT;
	status = ops->establish_clid(clp, cred);
	put_cred(cred);
	if (status != 0)
		return status;
	pnfs_destroy_all_layouts(clp);
	return 0;
}


// Source: nfs4state.c
// Lines 2000-2019
