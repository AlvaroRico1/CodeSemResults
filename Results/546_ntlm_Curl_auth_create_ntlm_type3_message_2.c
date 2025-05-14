// Source: curl/lib/vauth/ntlm.c
// Lines 510-517
CURLcode Curl_auth_create_ntlm_type3_message(struct Curl_easy *data,
                                             const char *userp,
                                             const char *passwdp,
                                             struct ntlmdata *ntlm,
                                             struct bufref *out)
{
  /* NTLM type-3 message structure:

          Index  Description            Content
            0    NTLMSSP Signature      Null-terminated ASCII "NTLMSSP"
                                        (0x4e544c4d53535000)
            8    NTLM Message Type      long (0x03000000)
           12    LM/LMv2 Response       security buffer
           20    NTLM/NTLMv2 Response   security buffer
           28    Target Name            security buffer
           36    User Name              security buffer
           44    Workstation Name       security buffer
          (52)   Session Key            security buffer (*)
          (60)   Flags                  long (*)
          (64)   OS Version Structure   8 bytes (*)
  52 (64) (72)   Start of data block
                                          (*) -> Optional
  */

  CURLcode result = CURLE_OK;
  size_t size;
  unsigned char ntlmbuf[NTLM_BUFSIZE];
  int lmrespoff;
  unsigned char lmresp[24]; /* fixed-size */
#ifdef USE_NTRESPONSES
  int ntrespoff;
  unsigned int ntresplen = 24;
  unsigned char ntresp[24]; /* fixed-size */
  unsigned char *ptr_ntresp = &ntresp[0];
  unsigned char *ntlmv2resp = NULL;
#endif
  bool unicode = (ntlm->flags & NTLMFLAG_NEGOTIATE_UNICODE) ? TRUE : FALSE;
  char host[HOSTNAME_MAX + 1] = "";
  const char *user;
  const char *domain = "";
  size_t hostoff = 0;
  size_t useroff = 0;
  size_t domoff = 0;
  size_t hostlen = 0;
  size_t userlen = 0;
  size_t domlen = 0;

  user = strchr(userp, '\\');
  if(!user)
    user = strchr(userp, '/');

  if(user) {
    domain = userp;
    domlen = (user - domain);
    user++;
  }
  else
    user = userp;

  userlen = strlen(user);

  /* Get the machine's un-qualified host name as NTLM doesn't like the fully
     qualified domain name */
  if(Curl_gethostname(host, sizeof(host))) {
    infof(data, "gethostname() failed, continuing without!");
    hostlen = 0;
  }
  else {
    hostlen = strlen(host);
  }

#if defined(USE_NTRESPONSES) && \
    (defined(USE_NTLM2SESSION) || defined(USE_NTLM_V2))
  /* We don't support NTLM2 or extended security if we don't have
     USE_NTRESPONSES */
  if(ntlm->flags & NTLMFLAG_NEGOTIATE_NTLM2_KEY) {
# if defined(USE_NTLM_V2)
    unsigned char ntbuffer[0x18];
    unsigned char entropy[8];
    unsigned char ntlmv2hash[0x18];

    /* Full NTLM version 2
       Although this cannot be negotiated, it is used here if available, as
       servers featuring extended security are likely supporting also
       NTLMv2. */
    result = Curl_rand(data, entropy, 8);
    if(result)
      return result;

    result = Curl_ntlm_core_mk_nt_hash(data, passwdp, ntbuffer);
    if(result)
      return result;

    result = Curl_ntlm_core_mk_ntlmv2_hash(user, userlen, domain, domlen,
                                           ntbuffer, ntlmv2hash);
    if(result)
      return result;

    /* LMv2 response */
    result = Curl_ntlm_core_mk_lmv2_resp(ntlmv2hash, entropy,
                                         &ntlm->nonce[0], lmresp);
    if(result)
      return result;

    /* NTLMv2 response */
    result = Curl_ntlm_core_mk_ntlmv2_resp(ntlmv2hash, entropy,
                                           ntlm, &ntlmv2resp, &ntresplen);
    if(result)
      return result;

    ptr_ntresp = ntlmv2resp;
# else /* defined(USE_NTLM_V2) */
    unsigned char ntbuffer[0x18];
    unsigned char tmp[0x18];
    unsigned char md5sum[MD5_DIGEST_LEN];
    unsigned char entropy[8];

    /* NTLM version 1 with extended security. */

    /* Need to create 8 bytes random data */
    result = Curl_rand(data, entropy, 8);
    if(result)
      return result;

    /* 8 bytes random data as challenge in lmresp */
    memcpy(lmresp, entropy, 8);

    /* Pad with zeros */
    memset(lmresp + 8, 0, 0x10);

    /* Fill tmp with challenge(nonce?) + entropy */
    memcpy(tmp, &ntlm->nonce[0], 8);
    memcpy(tmp + 8, entropy, 8);

    Curl_md5it(md5sum, tmp, 16);

    /* We shall only use the first 8 bytes of md5sum, but the des code in
       Curl_ntlm_core_lm_resp only encrypt the first 8 bytes */
    result = Curl_ntlm_core_mk_nt_hash(data, passwdp, ntbuffer);
    if(result)
      return result;

    Curl_ntlm_core_lm_resp(ntbuffer, md5sum, ntresp);

    /* End of NTLM2 Session code */
    /* NTLM v2 session security is a misnomer because it is not NTLM v2.
       It is NTLM v1 using the extended session security that is also
       in NTLM v2 */
# endif /* defined(USE_NTLM_V2) */
  }
  else
#endif
  {

#ifdef USE_NTRESPONSES
    unsigned char ntbuffer[0x18];
#endif
    unsigned char lmbuffer[0x18];

    /* NTLM version 1 */

#ifdef USE_NTRESPONSES
    result = Curl_ntlm_core_mk_nt_hash(data, passwdp, ntbuffer);
    if(result)
      return result;

    Curl_ntlm_core_lm_resp(ntbuffer, &ntlm->nonce[0], ntresp);
#endif

    result = Curl_ntlm_core_mk_lm_hash(data, passwdp, lmbuffer);
    if(result)
      return result;

    Curl_ntlm_core_lm_resp(lmbuffer, &ntlm->nonce[0], lmresp);
    ntlm->flags &= ~NTLMFLAG_NEGOTIATE_NTLM2_KEY;

    /* A safer but less compatible alternative is:
     *   Curl_ntlm_core_lm_resp(ntbuffer, &ntlm->nonce[0], lmresp);
     * See https://davenport.sourceforge.io/ntlm.html#ntlmVersion2 */
  }

  if(unicode) {
    domlen = domlen * 2;
    userlen = userlen * 2;
    hostlen = hostlen * 2;
  }

  lmrespoff = 64; /* size of the message header */
#ifdef USE_NTRESPONSES
  ntrespoff = lmrespoff + 0x18;
  domoff = ntrespoff + ntresplen;
#else
  domoff = lmrespoff + 0x18;
#endif
  useroff = domoff + domlen;
  hostoff = useroff + userlen;

  /* Create the big type-3 message binary blob */
  size = msnprintf((char *)ntlmbuf, NTLM_BUFSIZE,
                   NTLMSSP_SIGNATURE "%c"
                   "\x03%c%c%c"  /* 32-bit type = 3 */

                   "%c%c"  /* LanManager length */
                   "%c%c"  /* LanManager allocated space */
                   "%c%c"  /* LanManager offset */
                   "%c%c"  /* 2 zeroes */

                   "%c%c"  /* NT-response length */
                   "%c%c"  /* NT-response allocated space */
                   "%c%c"  /* NT-response offset */
                   "%c%c"  /* 2 zeroes */

                   "%c%c"  /* domain length */
                   "%c%c"  /* domain allocated space */
                   "%c%c"  /* domain name offset */
                   "%c%c"  /* 2 zeroes */

                   "%c%c"  /* user length */
                   "%c%c"  /* user allocated space */
                   "%c%c"  /* user offset */
                   "%c%c"  /* 2 zeroes */

                   "%c%c"  /* host length */
                   "%c%c"  /* host allocated space */
                   "%c%c"  /* host offset */
                   "%c%c"  /* 2 zeroes */

                   "%c%c"  /* session key length (unknown purpose) */
                   "%c%c"  /* session key allocated space (unknown purpose) */
                   "%c%c"  /* session key offset (unknown purpose) */
                   "%c%c"  /* 2 zeroes */

                   "%c%c%c%c",  /* flags */

                   /* domain string */
                   /* user string */
                   /* host string */
                   /* LanManager response */
                   /* NT response */

                   0,                /* zero termination */
                   0, 0, 0,          /* type-3 long, the 24 upper bits */

                   SHORTPAIR(0x18),  /* LanManager response length, twice */
                   SHORTPAIR(0x18),
                   SHORTPAIR(lmrespoff),
                   0x0, 0x0,

#ifdef USE_NTRESPONSES
                   SHORTPAIR(ntresplen),  /* NT-response length, twice */
                   SHORTPAIR(ntresplen),
                   SHORTPAIR(ntrespoff),
                   0x0, 0x0,
#else
                   0x0, 0x0,
                   0x0, 0x0,
                   0x0, 0x0,
                   0x0, 0x0,
#endif
                   SHORTPAIR(domlen),
                   SHORTPAIR(domlen),
                   SHORTPAIR(domoff),
                   0x0, 0x0,

                   SHORTPAIR(userlen),
                   SHORTPAIR(userlen),
                   SHORTPAIR(useroff),
                   0x0, 0x0,

                   SHORTPAIR(hostlen),
                   SHORTPAIR(hostlen),
                   SHORTPAIR(hostoff),
                   0x0, 0x0,

                   0x0, 0x0,
                   0x0, 0x0,
                   0x0, 0x0,
                   0x0, 0x0,

                   LONGQUARTET(ntlm->flags));

  DEBUGASSERT(size == 64);
  DEBUGASSERT(size == (size_t)lmrespoff);

  /* We append the binary hashes */
  if(size < (NTLM_BUFSIZE - 0x18)) {
    memcpy(&ntlmbuf[size], lmresp, 0x18);
    size += 0x18;
  }

  DEBUG_OUT({
    fprintf(stderr, "**** TYPE3 header lmresp=");
    ntlm_print_hex(stderr, (char *)&ntlmbuf[lmrespoff], 0x18);
  });

#ifdef USE_NTRESPONSES
  /* ntresplen + size should not be risking an integer overflow here */
  if(ntresplen + size > sizeof(ntlmbuf)) {
    failf(data, "incoming NTLM message too big");
    return CURLE_OUT_OF_MEMORY;
  }
  DEBUGASSERT(size == (size_t)ntrespoff);
  memcpy(&ntlmbuf[size], ptr_ntresp, ntresplen);
  size += ntresplen;

  DEBUG_OUT({
    fprintf(stderr, "\n   ntresp=");
    ntlm_print_hex(stderr, (char *)&ntlmbuf[ntrespoff], ntresplen);
  });

  free(ntlmv2resp);/* Free the dynamic buffer allocated for NTLMv2 */

#endif

  DEBUG_OUT({
    fprintf(stderr, "\n   flags=0x%02.2x%02.2x%02.2x%02.2x 0x%08.8x ",
            LONGQUARTET(ntlm->flags), ntlm->flags);
    ntlm_print_flags(stderr, ntlm->flags);
    fprintf(stderr, "\n****\n");
  });

  /* Make sure that the domain, user and host strings fit in the
     buffer before we copy them there. */
  if(size + userlen + domlen + hostlen >= NTLM_BUFSIZE) {
    failf(data, "user + domain + host name too big");
    return CURLE_OUT_OF_MEMORY;
  }

  DEBUGASSERT(size == domoff);
  if(unicode)
    unicodecpy(&ntlmbuf[size], domain, domlen / 2);
  else
    memcpy(&ntlmbuf[size], domain, domlen);

  size += domlen;

  DEBUGASSERT(size == useroff);
  if(unicode)
    unicodecpy(&ntlmbuf[size], user, userlen / 2);
  else
    memcpy(&ntlmbuf[size], user, userlen);

  size += userlen;

  DEBUGASSERT(size == hostoff);
  if(unicode)
    unicodecpy(&ntlmbuf[size], host, hostlen / 2);
  else
    memcpy(&ntlmbuf[size], host, hostlen);

  size += hostlen;

  /* Convert domain, user, and host to ASCII but leave the rest as-is */
  result = Curl_convert_to_network(data, (char *)&ntlmbuf[domoff],
                                   size - domoff);
  if(result)
    return CURLE_CONV_FAILED;

  /* Return the binary blob. */
  result = Curl_bufref_memdup(out, ntlmbuf, size);

  Curl_auth_cleanup_ntlm(ntlm);

  return result;
}
