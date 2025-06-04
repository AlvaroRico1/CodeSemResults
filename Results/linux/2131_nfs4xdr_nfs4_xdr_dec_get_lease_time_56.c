static int nfs4_xdr_dec_get_lease_time(struct rpc_rqst *rqstp,
				       struct xdr_stream *xdr,
				       void *data)
{
	struct nfs4_get_lease_time_res *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (!status)
		status = decode_sequence(xdr, &res->lr_seq_res, rqstp);
	if (!status)
		status = decode_putrootfh(xdr);
	if (!status)
		status = decode_fsinfo(xdr, res->lr_fsinfo);
	return status;
}


// Source: nfs4xdr.c
// Lines 7136-7152
