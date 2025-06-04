static void nfs4_xdr_enc_secinfo(struct rpc_rqst *req,
				struct xdr_stream *xdr,
				const void *data)
{
	const struct nfs4_secinfo_arg *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->dir_fh, &hdr);
	encode_secinfo(xdr, args->name, &hdr);
	encode_nops(&hdr);
}


// Source: nfs4xdr.c
// Lines 2823-2837
