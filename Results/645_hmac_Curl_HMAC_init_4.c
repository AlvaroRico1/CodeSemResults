// Source: curl/lib/hmac.c
// Lines 57-57
Curl_HMAC_init(const struct HMAC_params *hashparams,
               const unsigned char *key,
               unsigned int keylen)
{
  size_t i;
  struct HMAC_context *ctxt;
  unsigned char *hkey;
  unsigned char b;

  /* Create HMAC context. */
  i = sizeof(*ctxt) + 2 * hashparams->hmac_ctxtsize +
    hashparams->hmac_resultlen;
  ctxt = malloc(i);

  if(!ctxt)
    return ctxt;

  ctxt->hmac_hash = hashparams;
  ctxt->hmac_hashctxt1 = (void *) (ctxt + 1);
  ctxt->hmac_hashctxt2 = (void *) ((char *) ctxt->hmac_hashctxt1 +
      hashparams->hmac_ctxtsize);

  /* If the key is too long, replace it by its hash digest. */
  if(keylen > hashparams->hmac_maxkeylen) {
    (*hashparams->hmac_hinit)(ctxt->hmac_hashctxt1);
    (*hashparams->hmac_hupdate)(ctxt->hmac_hashctxt1, key, keylen);
    hkey = (unsigned char *) ctxt->hmac_hashctxt2 + hashparams->hmac_ctxtsize;
    (*hashparams->hmac_hfinal)(hkey, ctxt->hmac_hashctxt1);
    key = hkey;
    keylen = hashparams->hmac_resultlen;
  }

  /* Prime the two hash contexts with the modified key. */
  (*hashparams->hmac_hinit)(ctxt->hmac_hashctxt1);
  (*hashparams->hmac_hinit)(ctxt->hmac_hashctxt2);

  for(i = 0; i < keylen; i++) {
    b = (unsigned char)(*key ^ hmac_ipad);
    (*hashparams->hmac_hupdate)(ctxt->hmac_hashctxt1, &b, 1);
    b = (unsigned char)(*key++ ^ hmac_opad);
    (*hashparams->hmac_hupdate)(ctxt->hmac_hashctxt2, &b, 1);
  }

  for(; i < hashparams->hmac_maxkeylen; i++) {
    (*hashparams->hmac_hupdate)(ctxt->hmac_hashctxt1, &hmac_ipad, 1);
    (*hashparams->hmac_hupdate)(ctxt->hmac_hashctxt2, &hmac_opad, 1);
  }

  /* Done, return pointer to HMAC context. */
  return ctxt;
}
