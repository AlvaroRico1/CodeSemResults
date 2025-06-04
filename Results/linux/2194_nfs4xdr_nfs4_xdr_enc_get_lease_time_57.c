static void nfs4_xdr_enc_get_lease_time(struct rpc_rqst *req,
					struct xdr_stream *xdr,
					const void *data)
{
	const struct nfs4_get_lease_time_args *args = data;
	struct compound_hdr hdr = {
		.minorversion = nfs4_xdr_minorversion(&args->la_seq_args),
	};
	const u32 lease_bitmap[3] = { FATTR4_WORD0_LEASE_TIME };

	encode_compound_hdr(xdr, req, &hdr);
	encode_sequence(xdr, &args->la_seq_args, &hdr);
	encode_putrootfh(xdr, &hdr);
	encode_fsinfo(xdr, lease_bitmap, &hdr);
	encode_nops(&hdr);
}


// Source: nfs4xdr.c
// Lines 2967-2982
