static int nfs4_check_lease(struct nfs_client *clp)
{
	const struct cred *cred;
	const struct nfs4_state_maintenance_ops *ops =
		clp->cl_mvops->state_renewal_ops;
	int status;

	/* Is the client already known to have an expired lease? */
	if (test_bit(NFS4CLNT_LEASE_EXPIRED, &clp->cl_state))
		return 0;
	cred = ops->get_state_renewal_cred(clp);
	if (cred == NULL) {
		cred = nfs4_get_clid_cred(clp);
		status = -ENOKEY;
		if (cred == NULL)
			goto out;
	}
	status = ops->renew_lease(clp, cred);
	put_cred(cred);
	if (status == -ETIMEDOUT) {
		set_bit(NFS4CLNT_CHECK_LEASE, &clp->cl_state);
		return 0;
	}
out:
	return nfs4_recovery_handle_error(clp, status);
}


// Source: nfs4state.c
// Lines 1925-1950
