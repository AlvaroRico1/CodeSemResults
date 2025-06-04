void cf_hmac_init(cf_hmac_ctx *ctx,
                  const cf_chash *hash,
                  const uint8_t *key, size_t nkey)
{
  assert(ctx);
  assert(hash);

  mem_clean(ctx, sizeof *ctx);
  ctx->hash = hash;

  /* Prepare key: */
  uint8_t k[CF_CHASH_MAXBLK];

  /* Shorten long keys. */
  if (nkey > hash->blocksz)
  {
    /* Standard doesn't cover case where blocksz < hashsz.
     * FIPS186-1 seems to want to append a negative number of zero bytes.
     * In any case, we only have a k buffer of CF_CHASH_MAXBLK! */
    assert(hash->hashsz <= hash->blocksz);

    cf_hash(hash, key, nkey, k);
    key = k;
    nkey = hash->hashsz;
  }

  /* Right zero-pad short keys. */
  if (k != key)
    memcpy(k, key, nkey);
  if (hash->blocksz > nkey)
    memset(k + nkey, 0, hash->blocksz - nkey);

  /* Start inner hash computation */
  uint8_t blk[CF_CHASH_MAXBLK];

  xor_b8(blk, k, 0x36, hash->blocksz);
  hash->init(&ctx->inner);
  hash->update(&ctx->inner, blk, hash->blocksz);

  /* And outer. */
  xor_b8(blk, k, 0x5c, hash->blocksz);
  hash->init(&ctx->outer);
  hash->update(&ctx->outer, blk, hash->blocksz);

  mem_clean(blk, sizeof blk);
  mem_clean(k, sizeof k);
}


// Source: hmac.c
// Lines 23-69
