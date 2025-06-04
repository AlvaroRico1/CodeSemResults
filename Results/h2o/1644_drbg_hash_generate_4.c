static void hash_generate(const cf_chash *H,
                          uint8_t *data, size_t ndata, /* initialised with V */
                          void *out, size_t nout)
{
  cf_chash_ctx ctx;
  uint8_t w[CF_MAXHASH];
  uint8_t *bout = out;
  uint8_t one = 1;

  while (nout)
  {
    /* 4.1. w = Hash(data) */
    H->init(&ctx);
    H->update(&ctx, data, ndata);
    H->digest(&ctx, w);

    /* 4.2. W = W || w */
    size_t take = MIN(H->hashsz, nout);
    memcpy(bout, w, take);
    bout += take;
    nout -= take;

    /* 4.3. data = (data + 1) mod 2 ^ seedlen */
    add(data, ndata, &one, sizeof one);
  }
}


// Source: drbg.c
// Lines 136-161
