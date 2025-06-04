nfs4_xdr_dec_getacl(struct rpc_rqst *rqstp, struct xdr_stream *xdr,
		    void *data)
{
	struct nfs_getaclres *res = data;
	struct compound_hdr hdr;
	int status;

	if (res->acl_scratch != NULL) {
		void *p = page_address(res->acl_scratch);
		xdr_set_scratch_buffer(xdr, p, PAGE_SIZE);
	}
	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->seq_res, rqstp);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_getacl(xdr, rqstp, res);

out:
	return status;
}


// Source: nfs4xdr.c
// Lines 6374-6398
