// Source: curl/lib/mime.c
// Lines 1231-1231
CURLcode Curl_mime_duppart(curl_mimepart *dst, const curl_mimepart *src)
{
  curl_mime *mime;
  curl_mimepart *d;
  const curl_mimepart *s;
  CURLcode res = CURLE_OK;

  DEBUGASSERT(dst);

  /* Duplicate content. */
  switch(src->kind) {
  case MIMEKIND_NONE:
    break;
  case MIMEKIND_DATA:
    res = curl_mime_data(dst, src->data, (size_t) src->datasize);
    break;
  case MIMEKIND_FILE:
    res = curl_mime_filedata(dst, src->data);
    /* Do not abort duplication if file is not readable. */
    if(res == CURLE_READ_ERROR)
      res = CURLE_OK;
    break;
  case MIMEKIND_CALLBACK:
    res = curl_mime_data_cb(dst, src->datasize, src->readfunc,
                            src->seekfunc, src->freefunc, src->arg);
    break;
  case MIMEKIND_MULTIPART:
    /* No one knows about the cloned subparts, thus always attach ownership
       to the part. */
    mime = curl_mime_init(dst->easy);
    res = mime? curl_mime_subparts(dst, mime): CURLE_OUT_OF_MEMORY;

    /* Duplicate subparts. */
    for(s = ((curl_mime *) src->arg)->firstpart; !res && s; s = s->nextpart) {
      d = curl_mime_addpart(mime);
      res = d? Curl_mime_duppart(d, s): CURLE_OUT_OF_MEMORY;
    }
    break;
  default:  /* Invalid kind: should not occur. */
    res = CURLE_BAD_FUNCTION_ARGUMENT;  /* Internal error? */
    break;
  }

  /* Duplicate headers. */
  if(!res && src->userheaders) {
    struct curl_slist *hdrs = Curl_slist_duplicate(src->userheaders);

    if(!hdrs)
      res = CURLE_OUT_OF_MEMORY;
    else {
      /* No one but this procedure knows about the new header list,
         so always take ownership. */
      res = curl_mime_headers(dst, hdrs, TRUE);
      if(res)
        curl_slist_free_all(hdrs);
    }
  }

  if(!res) {
    /* Duplicate other fields. */
    dst->encoder = src->encoder;
    res = curl_mime_type(dst, src->mimetype);
  }
  if(!res)
    res = curl_mime_name(dst, src->name);
  if(!res)
    res = curl_mime_filename(dst, src->filename);

  /* If an error occurred, rollback. */
  if(res)
    Curl_mime_cleanpart(dst);

  return res;
}
