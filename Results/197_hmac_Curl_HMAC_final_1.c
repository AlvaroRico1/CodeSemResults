int Curl_HMAC_final(struct HMAC_context *ctxt, unsigned char *result)
{
  const struct HMAC_params *hashparams = ctxt->hmac_hash;

  /* Do not get result if called with a null parameter: only release
     storage. */

  if(!result)
    result = (unsigned char *) ctxt->hmac_hashctxt2 +
     ctxt->hmac_hash->hmac_ctxtsize;

  (*hashparams->hmac_hfinal)(result, ctxt->hmac_hashctxt1);
  (*hashparams->hmac_hupdate)(ctxt->hmac_hashctxt2,
   result, hashparams->hmac_resultlen);
  (*hashparams->hmac_hfinal)(result, ctxt->hmac_hashctxt2);
  free((char *) ctxt);
  return 0;
}


// Source: hmac.c
// Lines 114-131
