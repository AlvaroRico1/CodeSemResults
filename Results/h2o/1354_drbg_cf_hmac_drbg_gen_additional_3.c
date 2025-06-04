void cf_hmac_drbg_gen_additional(cf_hmac_drbg *ctx,
                                 const void *addnl, size_t naddnl,
                                 void *out, size_t nout)
{
  uint8_t *bout = out;

  while (nout != 0)
  {
    size_t take = MIN(MAX_DRBG_GENERATE, nout);
    hmac_drbg_generate(ctx, addnl, naddnl, bout, take);
    bout += take;
    nout -= take;

    /* Add additional data only once. */
    addnl = NULL;
    naddnl = 0;
  }
}


// Source: drbg.c
// Lines 398-415
