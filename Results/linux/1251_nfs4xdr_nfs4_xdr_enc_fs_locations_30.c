static void nfs4_xdr_enc_fs_locations(struct rpc_rqst *req,
				      struct xdr_stream *xdr,
				      const void *data)
{
	const struct nfs4_fs_locations_arg *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};
	uint32_t replen;

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	if (args->migration) {
		encode_putfh(xdr, args->fh, &hdr);
		replen = hdr.replen;
		encode_fs_locations(xdr, args->bitmask, &hdr);
		if (args->renew)
			encode_renew(xdr, args->clientid, &hdr);
	} else {
		encode_putfh(xdr, args->dir_fh, &hdr);
		encode_lookup(xdr, args->name, &hdr);
		replen = hdr.replen;
		encode_fs_locations(xdr, args->bitmask, &hdr);
	}

	rpc_prepare_reply_pages(req, (struct page **)&args->page, 0,
				PAGE_SIZE, replen + 1);
	encode_nops(&hdr);
}


// Source: nfs4xdr.c
// Lines 2790-2818
