static void nfs4_xdr_enc_server_caps(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs4_server_caps_arg *args = data;
	const u32 *bitmask = args->bitmask;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->fhandle, &hdr);
	encode_getattr(xdr, bitmask, NULL, 3, &hdr);
	encode_nops(&hdr);
}


// Source: nfs4xdr.c
// Lines 2696-2711
