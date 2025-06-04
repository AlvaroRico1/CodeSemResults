static void nfs4_xdr_enc_pathconf(struct rpc_rqst *req, struct xdr_stream *xdr,
				  const void *data)
{
	const struct nfs4_pathconf_arg *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->fh, &hdr);
	encode_getattr(xdr, nfs4_pathconf_bitmap, args->bitmask,
			ARRAY_SIZE(nfs4_pathconf_bitmap), &hdr);
	encode_nops(&hdr);
}


// Source: nfs4xdr.c
// Lines 2658-2672
