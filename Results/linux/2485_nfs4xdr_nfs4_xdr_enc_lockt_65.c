static void nfs4_xdr_enc_lockt(struct rpc_rqst *req, struct xdr_stream *xdr,
			       const void *data)
{
	const struct nfs_lockt_args *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->fh, &hdr);
	encode_lockt(xdr, args, &hdr);
	encode_nops(&hdr);
}


// Source: nfs4xdr.c
// Lines 2441-2454
