static int _nfs4_proc_fs_locations(struct rpc_clnt *client, struct inode *dir,
				   const struct qstr *name,
				   struct nfs4_fs_locations *fs_locations,
				   struct page *page)
{
	struct nfs_server *server = NFS_SERVER(dir);
	u32 bitmask[3];
	struct nfs4_fs_locations_arg args = {
		.dir_fh = NFS_FH(dir),
		.name = name,
		.page = page,
		.bitmask = bitmask,
	};
	struct nfs4_fs_locations_res res = {
		.fs_locations = fs_locations,
	};
	struct rpc_message msg = {
		.rpc_proc = &nfs4_procedures[NFSPROC4_CLNT_FS_LOCATIONS],
		.rpc_argp = &args,
		.rpc_resp = &res,
	};
	int status;

	dprintk("%s: start\n", __func__);

	bitmask[0] = nfs4_fattr_bitmap[0] | FATTR4_WORD0_FS_LOCATIONS;
	bitmask[1] = nfs4_fattr_bitmap[1];

	/* Ask for the fileid of the absent filesystem if mounted_on_fileid
	 * is not supported */
	if (NFS_SERVER(dir)->attr_bitmask[1] & FATTR4_WORD1_MOUNTED_ON_FILEID)
		bitmask[0] &= ~FATTR4_WORD0_FILEID;
	else
		bitmask[1] &= ~FATTR4_WORD1_MOUNTED_ON_FILEID;

	nfs_fattr_init(&fs_locations->fattr);
	fs_locations->server = server;
	fs_locations->nlocations = 0;
	status = nfs4_call_sync(client, server, &msg, &args.seq_args, &res.seq_res, 0);
	dprintk("%s: returned status = %d\n", __func__, status);
	return status;
}


// Source: nfs4proc.c
// Lines 7299-7340
