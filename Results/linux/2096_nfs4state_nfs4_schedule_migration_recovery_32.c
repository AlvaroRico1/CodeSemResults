int nfs4_schedule_migration_recovery(const struct nfs_server *server)
{
	struct nfs_client *clp = server->nfs_client;

	if (server->fh_expire_type != NFS4_FH_PERSISTENT) {
		pr_err("NFS: volatile file handles not supported (server %s)\n",
				clp->cl_hostname);
		return -NFS4ERR_IO;
	}

	if (test_bit(NFS_MIG_FAILED, &server->mig_status))
		return -NFS4ERR_IO;

	dprintk("%s: scheduling migration recovery for (%llx:%llx) on %s\n",
			__func__,
			(unsigned long long)server->fsid.major,
			(unsigned long long)server->fsid.minor,
			clp->cl_hostname);

	set_bit(NFS_MIG_IN_TRANSITION,
			&((struct nfs_server *)server)->mig_status);
	set_bit(NFS4CLNT_MOVED, &clp->cl_state);

	nfs4_schedule_state_manager(clp);
	return 0;
}


// Source: nfs4state.c
// Lines 1273-1298
