void cf_hash_drbg_sha256_gen_additional(cf_hash_drbg_sha256 *ctx,
                                        const void *addnl, size_t naddnl,
                                        void *out, size_t nout)
{
  uint8_t *bout = out;

  /* Generate output in requests of MAX_DRBG_GENERATE in size. */
  while (nout != 0)
  {
    size_t take = MIN(MAX_DRBG_GENERATE, nout);
    hash_gen_request(ctx, addnl, naddnl, bout, take);
    bout += take;
    nout -= take;

    /* Add additional data only once. */
    addnl = NULL;
    naddnl = 0;
  }
}


// Source: drbg.c
// Lines 207-225
