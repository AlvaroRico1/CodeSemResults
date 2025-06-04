static int crypto_cbcmac_digest_final(struct shash_desc *pdesc, u8 *out)
{
	struct crypto_shash *parent = pdesc->tfm;
	struct cbcmac_tfm_ctx *tctx = crypto_shash_ctx(parent);
	struct cbcmac_desc_ctx *ctx = shash_desc_ctx(pdesc);
	struct crypto_cipher *tfm = tctx->child;
	int bs = crypto_shash_digestsize(parent);
	u8 *dg = (u8 *)ctx + crypto_shash_descsize(parent) - bs;

	if (ctx->len)
		crypto_cipher_encrypt_one(tfm, dg, dg);

	memcpy(out, dg, bs);
	return 0;
}


// Source: ccm.c
// Lines 879-893
