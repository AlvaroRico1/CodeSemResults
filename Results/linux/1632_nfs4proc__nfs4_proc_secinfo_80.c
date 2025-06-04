static int _nfs4_proc_secinfo(struct inode *dir, const struct qstr *name, struct nfs4_secinfo_flavors *flavors, bool use_integrity)
{
	int status;
	struct nfs4_secinfo_arg args = {
		.dir_fh = NFS_FH(dir),
		.name   = name,
	};
	struct nfs4_secinfo_res res = {
		.flavors     = flavors,
	};
	struct rpc_message msg = {
		.rpc_proc = &nfs4_procedures[NFSPROC4_CLNT_SECINFO],
		.rpc_argp = &args,
		.rpc_resp = &res,
	};
	struct rpc_clnt *clnt = NFS_SERVER(dir)->client;
	const struct cred *cred = NULL;

	if (use_integrity) {
		clnt = NFS_SERVER(dir)->nfs_client->cl_rpcclient;
		cred = nfs4_get_clid_cred(NFS_SERVER(dir)->nfs_client);
		msg.rpc_cred = cred;
	}

	dprintk("NFS call  secinfo %s\n", name->name);

	nfs4_state_protect(NFS_SERVER(dir)->nfs_client,
		NFS_SP4_MACH_CRED_SECINFO, &clnt, &msg);

	status = nfs4_call_sync(clnt, NFS_SERVER(dir), &msg, &args.seq_args,
				&res.seq_res, RPC_TASK_NO_ROUND_ROBIN);
	dprintk("NFS reply  secinfo: %d\n", status);

	put_cred(cred);

	return status;
}


// Source: nfs4proc.c
// Lines 7645-7681
