CURLcode Curl_pin_peer_pubkey(struct Curl_easy *data,
                              const char *pinnedpubkey,
                              const unsigned char *pubkey, size_t pubkeylen)
{
  FILE *fp;
  unsigned char *buf = NULL, *pem_ptr = NULL;
  CURLcode result = CURLE_SSL_PINNEDPUBKEYNOTMATCH;

  /* if a path wasn't specified, don't pin */
  if(!pinnedpubkey)
    return CURLE_OK;
  if(!pubkey || !pubkeylen)
    return result;

  /* only do this if pinnedpubkey starts with "sha256//", length 8 */
  if(strncmp(pinnedpubkey, "sha256//", 8) == 0) {
    CURLcode encode;
    size_t encodedlen, pinkeylen;
    char *encoded, *pinkeycopy, *begin_pos, *end_pos;
    unsigned char *sha256sumdigest;

    if(!Curl_ssl->sha256sum) {
      /* without sha256 support, this cannot match */
      return result;
    }

    /* compute sha256sum of public key */
    sha256sumdigest = malloc(CURL_SHA256_DIGEST_LENGTH);
    if(!sha256sumdigest)
      return CURLE_OUT_OF_MEMORY;
    encode = Curl_ssl->sha256sum(pubkey, pubkeylen,
                        sha256sumdigest, CURL_SHA256_DIGEST_LENGTH);

    if(encode != CURLE_OK)
      return encode;

    encode = Curl_base64_encode(data, (char *)sha256sumdigest,
                                CURL_SHA256_DIGEST_LENGTH, &encoded,
                                &encodedlen);
    Curl_safefree(sha256sumdigest);

    if(encode)
      return encode;

    infof(data, " public key hash: sha256//%s", encoded);

    /* it starts with sha256//, copy so we can modify it */
    pinkeylen = strlen(pinnedpubkey) + 1;
    pinkeycopy = malloc(pinkeylen);
    if(!pinkeycopy) {
      Curl_safefree(encoded);
      return CURLE_OUT_OF_MEMORY;
    }
    memcpy(pinkeycopy, pinnedpubkey, pinkeylen);
    /* point begin_pos to the copy, and start extracting keys */
    begin_pos = pinkeycopy;
    do {
      end_pos = strstr(begin_pos, ";sha256//");
      /*
       * if there is an end_pos, null terminate,
       * otherwise it'll go to the end of the original string
       */
      if(end_pos)
        end_pos[0] = '\0';

      /* compare base64 sha256 digests, 8 is the length of "sha256//" */
      if(encodedlen == strlen(begin_pos + 8) &&
         !memcmp(encoded, begin_pos + 8, encodedlen)) {
        result = CURLE_OK;
        break;
      }

      /*
       * change back the null-terminator we changed earlier,
       * and look for next begin
       */
      if(end_pos) {
        end_pos[0] = ';';
        begin_pos = strstr(end_pos, "sha256//");
      }
    } while(end_pos && begin_pos);
    Curl_safefree(encoded);
    Curl_safefree(pinkeycopy);
    return result;
  }


// Source: vtls.c
// Lines 928-1012
