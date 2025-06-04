static void nfs4_xdr_enc_readlink(struct rpc_rqst *req, struct xdr_stream *xdr,
				  const void *data)
{
	const struct nfs4_readlink *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->fh, &hdr);
	encode_readlink(xdr, args, req, &hdr);

	rpc_prepare_reply_pages(req, args->pages, args->pgbase,
				args->pglen, hdr.replen);
	encode_nops(&hdr);
}


// Source: nfs4xdr.c
// Lines 2491-2507
