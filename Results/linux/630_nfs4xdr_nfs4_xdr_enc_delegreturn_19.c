static void nfs4_xdr_enc_delegreturn(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs4_delegreturnargs *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->fhandle, &hdr);
	if (args->lr_args)
		encode_layoutreturn(xdr, args->lr_args, &hdr);
	if (args->bitmask)
		encode_getfattr(xdr, args->bitmask, &hdr);
	encode_delegreturn(xdr, args->stateid, &hdr);
	encode_nops(&hdr);
}


// Source: nfs4xdr.c
// Lines 2767-2785
