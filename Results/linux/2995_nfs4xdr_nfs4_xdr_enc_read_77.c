static void nfs4_xdr_enc_read(struct rpc_rqst *req, struct xdr_stream *xdr,
			      const void *data)
{
	const struct nfs_pgio_args *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->fh, &hdr);
	encode_read(xdr, args, &hdr);

	rpc_prepare_reply_pages(req, args->pages, args->pgbase,
				args->count, hdr.replen);
	req->rq_rcv_buf.flags |= XDRBUF_READ;
	encode_nops(&hdr);
}


// Source: nfs4xdr.c
// Lines 2533-2550
