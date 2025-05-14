// Source: curl/lib/vauth/digest.c
// Lines 667-683
static CURLcode auth_create_digest_http_message(
                  struct Curl_easy *data,
                  const char *userp,
                  const char *passwdp,
                  const unsigned char *request,
                  const unsigned char *uripath,
                  struct digestdata *digest,
                  char **outptr, size_t *outlen,
                  void (*convert_to_ascii)(unsigned char *, unsigned char *),
                  void (*hash)(unsigned char *, const unsigned char *,
                               const size_t))
{
  CURLcode result;
  unsigned char hashbuf[32]; /* 32 bytes/256 bits */
  unsigned char request_digest[65];
  unsigned char ha1[65];    /* 64 digits and 1 zero byte */
  unsigned char ha2[65];    /* 64 digits and 1 zero byte */
  char userh[65];
  char *cnonce = NULL;
  size_t cnonce_sz = 0;
  char *userp_quoted;
  char *response = NULL;
  char *hashthis = NULL;
  char *tmp = NULL;

  if(!digest->nc)
    digest->nc = 1;

  if(!digest->cnonce) {
    char cnoncebuf[33];
    result = Curl_rand_hex(data, (unsigned char *)cnoncebuf,
                           sizeof(cnoncebuf));
    if(result)
      return result;

    result = Curl_base64_encode(data, cnoncebuf, strlen(cnoncebuf),
                                &cnonce, &cnonce_sz);
    if(result)
      return result;

    digest->cnonce = cnonce;
  }

  if(digest->userhash) {
    hashthis = aprintf("%s:%s", userp, digest->realm);
    if(!hashthis)
      return CURLE_OUT_OF_MEMORY;

    CURL_OUTPUT_DIGEST_CONV(data, hashthis);
    hash(hashbuf, (unsigned char *) hashthis, strlen(hashthis));
    free(hashthis);
    convert_to_ascii(hashbuf, (unsigned char *)userh);
  }

  /*
    If the algorithm is "MD5" or unspecified (which then defaults to MD5):

      A1 = unq(username-value) ":" unq(realm-value) ":" passwd

    If the algorithm is "MD5-sess" then:

      A1 = H(unq(username-value) ":" unq(realm-value) ":" passwd) ":"
           unq(nonce-value) ":" unq(cnonce-value)
  */

  hashthis = aprintf("%s:%s:%s", digest->userhash ? userh : userp,
                                 digest->realm, passwdp);
  if(!hashthis)
    return CURLE_OUT_OF_MEMORY;

  CURL_OUTPUT_DIGEST_CONV(data, hashthis); /* convert on non-ASCII machines */
  hash(hashbuf, (unsigned char *) hashthis, strlen(hashthis));
  free(hashthis);
  convert_to_ascii(hashbuf, ha1);

  if(digest->algo == CURLDIGESTALGO_MD5SESS ||
     digest->algo == CURLDIGESTALGO_SHA256SESS ||
     digest->algo == CURLDIGESTALGO_SHA512_256SESS) {
    /* nonce and cnonce are OUTSIDE the hash */
    tmp = aprintf("%s:%s:%s", ha1, digest->nonce, digest->cnonce);
    if(!tmp)
      return CURLE_OUT_OF_MEMORY;

    CURL_OUTPUT_DIGEST_CONV(data, tmp); /* Convert on non-ASCII machines */
    hash(hashbuf, (unsigned char *) tmp, strlen(tmp));
    free(tmp);
    convert_to_ascii(hashbuf, ha1);
  }

  /*
    If the "qop" directive's value is "auth" or is unspecified, then A2 is:

      A2 = Method ":" digest-uri-value

    If the "qop" value is "auth-int", then A2 is:

      A2 = Method ":" digest-uri-value ":" H(entity-body)

    (The "Method" value is the HTTP request method as specified in section
    5.1.1 of RFC 2616)
  */

  hashthis = aprintf("%s:%s", request, uripath);
  if(!hashthis)
    return CURLE_OUT_OF_MEMORY;

  if(digest->qop && strcasecompare(digest->qop, "auth-int")) {
    /* We don't support auth-int for PUT or POST */
    char hashed[65];
    char *hashthis2;

    hash(hashbuf, (const unsigned char *)"", 0);
    convert_to_ascii(hashbuf, (unsigned char *)hashed);

    hashthis2 = aprintf("%s:%s", hashthis, hashed);
    free(hashthis);
    hashthis = hashthis2;
  }

  if(!hashthis)
    return CURLE_OUT_OF_MEMORY;

  CURL_OUTPUT_DIGEST_CONV(data, hashthis); /* convert on non-ASCII machines */
  hash(hashbuf, (unsigned char *) hashthis, strlen(hashthis));
  free(hashthis);
  convert_to_ascii(hashbuf, ha2);

  if(digest->qop) {
    hashthis = aprintf("%s:%s:%08x:%s:%s:%s", ha1, digest->nonce, digest->nc,
                       digest->cnonce, digest->qop, ha2);
  }
  else {
    hashthis = aprintf("%s:%s:%s", ha1, digest->nonce, ha2);
  }

  if(!hashthis)
    return CURLE_OUT_OF_MEMORY;

  CURL_OUTPUT_DIGEST_CONV(data, hashthis); /* convert on non-ASCII machines */
  hash(hashbuf, (unsigned char *) hashthis, strlen(hashthis));
  free(hashthis);
  convert_to_ascii(hashbuf, request_digest);

  /* For test case 64 (snooped from a Mozilla 1.3a request)

     Authorization: Digest username="testuser", realm="testrealm", \
     nonce="1053604145", uri="/64", response="c55f7f30d83d774a3d2dcacf725abaca"

     Digest parameters are all quoted strings.  Username which is provided by
     the user will need double quotes and backslashes within it escaped.  For
     the other fields, this shouldn't be an issue.  realm, nonce, and opaque
     are copied as is from the server, escapes and all.  cnonce is generated
     with web-safe characters.  uri is already percent encoded.  nc is 8 hex
     characters.  algorithm and qop with standard values only contain web-safe
     characters.
  */
  userp_quoted = auth_digest_string_quoted(digest->userhash ? userh : userp);
  if(!userp_quoted)
    return CURLE_OUT_OF_MEMORY;

  if(digest->qop) {
    response = aprintf("username=\"%s\", "
                       "realm=\"%s\", "
                       "nonce=\"%s\", "
                       "uri=\"%s\", "
                       "cnonce=\"%s\", "
                       "nc=%08x, "
                       "qop=%s, "
                       "response=\"%s\"",
                       userp_quoted,
                       digest->realm,
                       digest->nonce,
                       uripath,
                       digest->cnonce,
                       digest->nc,
                       digest->qop,
                       request_digest);

    if(strcasecompare(digest->qop, "auth"))
      digest->nc++; /* The nc (from RFC) has to be a 8 hex digit number 0
                       padded which tells to the server how many times you are
                       using the same nonce in the qop=auth mode */
  }
  else {
    response = aprintf("username=\"%s\", "
                       "realm=\"%s\", "
                       "nonce=\"%s\", "
                       "uri=\"%s\", "
                       "response=\"%s\"",
                       userp_quoted,
                       digest->realm,
                       digest->nonce,
                       uripath,
                       request_digest);
  }
  free(userp_quoted);
  if(!response)
    return CURLE_OUT_OF_MEMORY;

  /* Add the optional fields */
  if(digest->opaque) {
    /* Append the opaque */
    tmp = aprintf("%s, opaque=\"%s\"", response, digest->opaque);
    free(response);
    if(!tmp)
      return CURLE_OUT_OF_MEMORY;

    response = tmp;
  }

  if(digest->algorithm) {
    /* Append the algorithm */
    tmp = aprintf("%s, algorithm=%s", response, digest->algorithm);
    free(response);
    if(!tmp)
      return CURLE_OUT_OF_MEMORY;

    response = tmp;
  }

  if(digest->userhash) {
    /* Append the userhash */
    tmp = aprintf("%s, userhash=true", response);
    free(response);
    if(!tmp)
      return CURLE_OUT_OF_MEMORY;

    response = tmp;
  }

  /* Return the output */
  *outptr = response;
  *outlen = strlen(response);

  return CURLE_OK;
}
