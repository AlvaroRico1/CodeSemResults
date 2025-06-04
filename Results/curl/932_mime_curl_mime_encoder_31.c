CURLcode curl_mime_encoder(curl_mimepart *part, const char *encoding)
{
  CURLcode result = CURLE_BAD_FUNCTION_ARGUMENT;
  const struct mime_encoder *mep;

  if(!part)
    return result;

  part->encoder = NULL;

  if(!encoding)
    return CURLE_OK;    /* Removing current encoder. */

  for(mep = encoders; mep->name; mep++)
    if(strcasecompare(encoding, mep->name)) {
      part->encoder = mep;
      result = CURLE_OK;
    }

  return result;
}


// Source: mime.c
// Lines 1503-1523
