static int ghash_setkey(struct crypto_shash *tfm,
			const u8 *key, unsigned int keylen)
{
	struct ghash_ctx *ctx = crypto_shash_ctx(tfm);
	be128 k;

	if (keylen != GHASH_BLOCK_SIZE) {
		crypto_shash_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	if (ctx->gf128)
		gf128mul_free_4k(ctx->gf128);

	BUILD_BUG_ON(sizeof(k) != GHASH_BLOCK_SIZE);
	memcpy(&k, key, GHASH_BLOCK_SIZE); /* avoid violating alignment rules */
	ctx->gf128 = gf128mul_init_4k_lle(&k);
	memzero_explicit(&k, GHASH_BLOCK_SIZE);

	if (!ctx->gf128)
		return -ENOMEM;

	return 0;
}


// Source: ghash-generic.c
// Lines 30-53
