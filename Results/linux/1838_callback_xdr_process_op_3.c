static __be32 process_op(int nop, struct svc_rqst *rqstp,
		struct xdr_stream *xdr_in, void *argp,
		struct xdr_stream *xdr_out, void *resp,
		struct cb_process_state *cps)
{
	struct callback_op *op = &callback_ops[0];
	unsigned int op_nr;
	__be32 status;
	long maxlen;
	__be32 res;

	status = decode_op_hdr(xdr_in, &op_nr);
	if (unlikely(status))
		return status;

	switch (cps->minorversion) {
	case 0:
		status = preprocess_nfs4_op(op_nr, &op);
		break;
	case 1:
		status = preprocess_nfs41_op(nop, op_nr, &op);
		break;
	case 2:
		status = preprocess_nfs42_op(nop, op_nr, &op);
		break;
	default:
		status = htonl(NFS4ERR_MINOR_VERS_MISMATCH);
	}

	if (status == htonl(NFS4ERR_OP_ILLEGAL))
		op_nr = OP_CB_ILLEGAL;
	if (status)
		goto encode_hdr;

	if (cps->drc_status) {
		status = cps->drc_status;
		goto encode_hdr;
	}

	maxlen = xdr_out->end - xdr_out->p;
	if (maxlen > 0 && maxlen < PAGE_SIZE) {
		status = op->decode_args(rqstp, xdr_in, argp);
		if (likely(status == 0))
			status = op->process_op(argp, resp, cps);
	} else
		status = htonl(NFS4ERR_RESOURCE);

encode_hdr:
	res = encode_op_hdr(xdr_out, op_nr, status);
	if (unlikely(res))
		return res;
	if (op->encode_res != NULL && status == 0)
		status = op->encode_res(rqstp, xdr_out, resp);
	return status;
}


// Source: callback_xdr.c
// Lines 865-919
