static void nfs3_xdr_enc_getacl3args(struct rpc_rqst *req,
				     struct xdr_stream *xdr,
				     const void *data)
{
	const struct nfs3_getaclargs *args = data;

	encode_nfs_fh3(xdr, args->fh);
	encode_uint32(xdr, args->mask);
	if (args->mask & (NFS_ACL | NFS_DFACL)) {
		rpc_prepare_reply_pages(req, args->pages, 0,
					NFSACL_MAXPAGES << PAGE_SHIFT,
					ACL3_getaclres_sz);
		req->rq_rcv_buf.flags |= XDRBUF_SPARSE_PAGES;
	}
}


// Source: nfs3xdr.c
// Lines 1324-1338
