static int nfs4_xdr_dec_fs_locations(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     void *data)
{
	struct nfs4_fs_locations_res *res = data;
	struct compound_hdr hdr;
	int status;

	status = decode_compound_hdr(xdr, &hdr);
	if (status)
		goto out;
	status = decode_sequence(xdr, &res->seq_res, req);
	if (status)
		goto out;
	status = decode_putfh(xdr);
	if (status)
		goto out;
	if (res->migration) {
		xdr_enter_page(xdr, PAGE_SIZE);
		status = decode_getfattr_generic(xdr,
					&res->fs_locations->fattr,
					 NULL, res->fs_locations,
					 NULL, res->fs_locations->server);
		if (status)
			goto out;
		if (res->renew)
			status = decode_renew(xdr);
	} else {
		status = decode_lookup(xdr);
		if (status)
			goto out;
		xdr_enter_page(xdr, PAGE_SIZE);
		status = decode_getfattr_generic(xdr,
					&res->fs_locations->fattr,
					 NULL, res->fs_locations,
					 NULL, res->fs_locations->server);
	}
out:
	return status;
}


// Source: nfs4xdr.c
// Lines 6939-6978
