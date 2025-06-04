static int scomp_acomp_comp_decomp(struct acomp_req *req, int dir)
{
	struct crypto_acomp *tfm = crypto_acomp_reqtfm(req);
	void **tfm_ctx = acomp_tfm_ctx(tfm);
	struct crypto_scomp *scomp = *tfm_ctx;
	void **ctx = acomp_request_ctx(req);
	struct scomp_scratch *scratch;
	int ret;

	if (!req->src || !req->slen || req->slen > SCOMP_SCRATCH_SIZE)
		return -EINVAL;

	if (req->dst && !req->dlen)
		return -EINVAL;

	if (!req->dlen || req->dlen > SCOMP_SCRATCH_SIZE)
		req->dlen = SCOMP_SCRATCH_SIZE;

	scratch = raw_cpu_ptr(&scomp_scratch);
	spin_lock(&scratch->lock);

	scatterwalk_map_and_copy(scratch->src, req->src, 0, req->slen, 0);
	if (dir)
		ret = crypto_scomp_compress(scomp, scratch->src, req->slen,
					    scratch->dst, &req->dlen, *ctx);
	else
		ret = crypto_scomp_decompress(scomp, scratch->src, req->slen,
					      scratch->dst, &req->dlen, *ctx);
	if (!ret) {
		if (!req->dst) {
			req->dst = sgl_alloc(req->dlen, GFP_ATOMIC, NULL);
			if (!req->dst) {
				ret = -ENOMEM;
				goto out;
			}
		}
		scatterwalk_map_and_copy(scratch->dst, req->dst, 0, req->dlen,
					 1);
	}
out:
	spin_unlock(&scratch->lock);
	return ret;
}


// Source: scompress.c
// Lines 120-162
