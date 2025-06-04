static int nfs4_handle_lease_moved(struct nfs_client *clp)
{
	const struct nfs4_state_maintenance_ops *ops =
				clp->cl_mvops->state_renewal_ops;
	struct nfs_server *server;
	const struct cred *cred;

	dprintk("%s: lease moved reported on \"%s\"\n", __func__,
			clp->cl_hostname);

	cred = ops->get_state_renewal_cred(clp);
	if (cred == NULL)
		return -NFS4ERR_NOENT;

	clp->cl_mig_gen++;
restart:
	rcu_read_lock();
	list_for_each_entry_rcu(server, &clp->cl_superblocks, client_link) {
		struct inode *inode;
		int status;

		if (server->mig_gen == clp->cl_mig_gen)
			continue;
		server->mig_gen = clp->cl_mig_gen;

		rcu_read_unlock();

		inode = d_inode(server->super->s_root);
		status = nfs4_proc_fsid_present(inode, cred);
		if (status != -NFS4ERR_MOVED)
			goto restart;	/* wasn't this one */
		if (nfs4_try_migration(server, cred) == -NFS4ERR_LEASE_MOVED)
			goto restart;	/* there are more */
		goto out;
	}
	rcu_read_unlock();

out:
	put_cred(cred);
	return 0;
}


// Source: nfs4state.c
// Lines 2172-2212
