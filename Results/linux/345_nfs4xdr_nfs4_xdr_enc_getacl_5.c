static void nfs4_xdr_enc_getacl(struct rpc_rqst *req, struct xdr_stream *xdr,
				const void *data)
{
	const struct nfs_getaclargs *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->seq_args),
	};
	const __u32 nfs4_acl_bitmap[1] = {
		[0] = FATTR4_WORD0_ACL,
	};
	uint32_t replen;

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->seq_args, &hdr);
	encode_putfh(xdr, args->fh, &hdr);
	replen = hdr.replen + op_decode_hdr_maxsz;
	encode_getattr(xdr, nfs4_acl_bitmap, NULL,
			ARRAY_SIZE(nfs4_acl_bitmap), &hdr);

	rpc_prepare_reply_pages(req, args->acl_pages, 0,
				args->acl_len, replen + 1);
	encode_nops(&hdr);
}


// Source: nfs4xdr.c
// Lines 2574-2596
