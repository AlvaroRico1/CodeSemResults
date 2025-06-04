static void nfs_pgio_rpcsetup(struct nfs_pgio_header *hdr,
			      unsigned int count,
			      int how, struct nfs_commit_info *cinfo)
{
	struct nfs_page *req = hdr->req;

	/* Set up the RPC argument and reply structs
	 * NB: take care not to mess about with hdr->commit et al. */

	hdr->args.fh     = NFS_FH(hdr->inode);
	hdr->args.offset = req_offset(req);
	/* pnfs_set_layoutcommit needs this */
	hdr->mds_offset = hdr->args.offset;
	hdr->args.pgbase = req->wb_pgbase;
	hdr->args.pages  = hdr->page_array.pagevec;
	hdr->args.count  = count;
	hdr->args.context = get_nfs_open_context(nfs_req_openctx(req));
	hdr->args.lock_context = req->wb_lock_context;
	hdr->args.stable  = NFS_UNSTABLE;
	switch (how & (FLUSH_STABLE | FLUSH_COND_STABLE)) {
	case 0:
		break;
	case FLUSH_COND_STABLE:
		if (nfs_reqs_to_commit(cinfo))
			break;
		/* fall through */
	default:
		hdr->args.stable = NFS_FILE_SYNC;
	}

	hdr->res.fattr   = &hdr->fattr;
	hdr->res.count   = 0;
	hdr->res.eof     = 0;
	hdr->res.verf    = &hdr->verf;
	nfs_fattr_init(&hdr->fattr);
}


// Source: pagelist.c
// Lines 562-597
