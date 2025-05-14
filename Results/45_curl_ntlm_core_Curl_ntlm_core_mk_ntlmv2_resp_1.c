// Source: curl/lib/curl_ntlm_core.c
// Lines 614-633
CURLcode Curl_ntlm_core_mk_ntlmv2_resp(unsigned char *ntlmv2hash,
                                       unsigned char *challenge_client,
                                       struct ntlmdata *ntlm,
                                       unsigned char **ntresp,
                                       unsigned int *ntresp_len)
{
/* NTLMv2 response structure :
------------------------------------------------------------------------------
0     HMAC MD5         16 bytes
------BLOB--------------------------------------------------------------------
16    Signature        0x01010000
20    Reserved         long (0x00000000)
24    Timestamp        LE, 64-bit signed value representing the number of
                       tenths of a microsecond since January 1, 1601.
32    Client Nonce     8 bytes
40    Unknown          4 bytes
44    Target Info      N bytes (from the type-2 message)
44+N  Unknown          4 bytes
------------------------------------------------------------------------------
*/

  unsigned int len = 0;
  unsigned char *ptr = NULL;
  unsigned char hmac_output[HMAC_MD5_LENGTH];
  struct ms_filetime tw;

  CURLcode result = CURLE_OK;

  /* Calculate the timestamp */
#ifdef DEBUGBUILD
  char *force_timestamp = getenv("CURL_FORCETIME");
  if(force_timestamp)
    time2filetime(&tw, (time_t) 0);
  else
#endif
    time2filetime(&tw, time(NULL));

  /* Calculate the response len */
  len = HMAC_MD5_LENGTH + NTLMv2_BLOB_LEN;

  /* Allocate the response */
  ptr = calloc(1, len);
  if(!ptr)
    return CURLE_OUT_OF_MEMORY;

  /* Create the BLOB structure */
  msnprintf((char *)ptr + HMAC_MD5_LENGTH, NTLMv2_BLOB_LEN,
            "%c%c%c%c"           /* NTLMv2_BLOB_SIGNATURE */
            "%c%c%c%c"           /* Reserved = 0 */
            "%c%c%c%c%c%c%c%c",  /* Timestamp */
            NTLMv2_BLOB_SIGNATURE[0], NTLMv2_BLOB_SIGNATURE[1],
            NTLMv2_BLOB_SIGNATURE[2], NTLMv2_BLOB_SIGNATURE[3],
            0, 0, 0, 0,
            LONGQUARTET(tw.dwLowDateTime), LONGQUARTET(tw.dwHighDateTime));

  memcpy(ptr + 32, challenge_client, 8);
  memcpy(ptr + 44, ntlm->target_info, ntlm->target_info_len);

  /* Concatenate the Type 2 challenge with the BLOB and do HMAC MD5 */
  memcpy(ptr + 8, &ntlm->nonce[0], 8);
  result = Curl_hmacit(Curl_HMAC_MD5, ntlmv2hash, HMAC_MD5_LENGTH, ptr + 8,
                    NTLMv2_BLOB_LEN + 8, hmac_output);
  if(result) {
    free(ptr);
    return result;
  }

  /* Concatenate the HMAC MD5 output  with the BLOB */
  memcpy(ptr, hmac_output, HMAC_MD5_LENGTH);

  /* Return the response */
  *ntresp = ptr;
  *ntresp_len = len;

  return result;
}
