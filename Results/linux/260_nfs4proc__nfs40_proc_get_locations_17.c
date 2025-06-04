static int _nfs40_proc_get_locations(struct inode *inode,
				     struct nfs4_fs_locations *locations,
				     struct page *page, const struct cred *cred)
{
	struct nfs_server *server = NFS_SERVER(inode);
	struct rpc_clnt *clnt = server->client;
	u32 bitmask[2] = {
		[0] = FATTR4_WORD0_FSID | FATTR4_WORD0_FS_LOCATIONS,
	};
	struct nfs4_fs_locations_arg args = {
		.clientid	= server->nfs_client->cl_clientid,
		.fh		= NFS_FH(inode),
		.page		= page,
		.bitmask	= bitmask,
		.migration	= 1,		/* skip LOOKUP */
		.renew		= 1,		/* append RENEW */
	};
	struct nfs4_fs_locations_res res = {
		.fs_locations	= locations,
		.migration	= 1,
		.renew		= 1,
	};
	struct rpc_message msg = {
		.rpc_proc	= &nfs4_procedures[NFSPROC4_CLNT_FS_LOCATIONS],
		.rpc_argp	= &args,
		.rpc_resp	= &res,
		.rpc_cred	= cred,
	};
	unsigned long now = jiffies;
	int status;

	nfs_fattr_init(&locations->fattr);
	locations->server = server;
	locations->nlocations = 0;

	nfs4_init_sequence(&args.seq_args, &res.seq_res, 0, 1);
	status = nfs4_call_sync_sequence(clnt, server, &msg,
					&args.seq_args, &res.seq_res);
	if (status)
		return status;

	renew_lease(server, now);
	return 0;
}


// Source: nfs4proc.c
// Lines 7368-7411
