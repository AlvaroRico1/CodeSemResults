static int nfs4_xdr_dec_link(struct rpc_rqst *rqstp, struct xdr_stream *xdr,
			     void *data)
{
	struct nfs4_link_res *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->seq_res, rqstp);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_savefh(xdr);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	status = decode_link(xdr, &res->cinfo);
	if (status)
		goto out;
	/*
	 * Note order: OP_LINK leaves the directory as the current
	 *             filehandle.
	 */
	status = decode_restorefh(xdr);
	if (status)
		goto out;
	decode_getfattr_label(xdr, res->fattr, res->label, res->server);
out:
	return status;
}


// Source: nfs4xdr.c
// Lines 6227-6262
