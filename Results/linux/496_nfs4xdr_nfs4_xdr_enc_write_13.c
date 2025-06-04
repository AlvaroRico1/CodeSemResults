static void nfs4_xdr_enc_write(struct rpc_rqst *req, struct xdr_stream *xdr,
			       const void *data)
{
	const struct nfs_pgio_args *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->fh, &hdr);
	encode_write(xdr, args, &hdr);
	req->rq_snd_buf.flags |= XDRBUF_WRITE;
	if (args->bitmask)
		encode_getfattr(xdr, args->bitmask, &hdr);
	encode_nops(&hdr);
}


// Source: nfs4xdr.c
// Lines 2601-2617
