CURLcode Curl_mime_prepare_headers(curl_mimepart *part,
                                   const char *contenttype,
                                   const char *disposition,
                                   enum mimestrategy strategy)
{
  curl_mime *mime = NULL;
  const char *boundary = NULL;
  char *customct;
  const char *cte = NULL;
  CURLcode ret = CURLE_OK;

  /* Get rid of previously prepared headers. */
  curl_slist_free_all(part->curlheaders);
  part->curlheaders = NULL;

  /* Be sure we won't access old headers later. */
  if(part->state.state == MIMESTATE_CURLHEADERS)
    mimesetstate(&part->state, MIMESTATE_CURLHEADERS, NULL);

  /* Check if content type is specified. */
  customct = part->mimetype;
  if(!customct)
    customct = search_header(part->userheaders, "Content-Type");
  if(customct)
    contenttype = customct;

  /* If content type is not specified, try to determine it. */
  if(!contenttype) {
    switch(part->kind) {
    case MIMEKIND_MULTIPART:
      contenttype = MULTIPART_CONTENTTYPE_DEFAULT;
      break;
    case MIMEKIND_FILE:
      contenttype = Curl_mime_contenttype(part->filename);
      if(!contenttype)
        contenttype = Curl_mime_contenttype(part->data);
      if(!contenttype && part->filename)
        contenttype = FILE_CONTENTTYPE_DEFAULT;
      break;
    default:
      contenttype = Curl_mime_contenttype(part->filename);
      break;
    }
  }

  if(part->kind == MIMEKIND_MULTIPART) {
    mime = (curl_mime *) part->arg;
    if(mime)
      boundary = mime->boundary;
  }
  else if(contenttype && !customct &&
          content_type_match(contenttype, "text/plain"))
    if(strategy == MIMESTRATEGY_MAIL || !part->filename)
      contenttype = NULL;

  /* Issue content-disposition header only if not already set by caller. */
  if(!search_header(part->userheaders, "Content-Disposition")) {
    if(!disposition)
      if(part->filename || part->name ||
        (contenttype && !strncasecompare(contenttype, "multipart/", 10)))
          disposition = DISPOSITION_DEFAULT;
    if(disposition && curl_strequal(disposition, "attachment") &&
     !part->name && !part->filename)
      disposition = NULL;
    if(disposition) {
      char *name = NULL;
      char *filename = NULL;

      if(part->name) {
        name = escape_string(part->name);
        if(!name)
          ret = CURLE_OUT_OF_MEMORY;
      }
      if(!ret && part->filename) {
        filename = escape_string(part->filename);
        if(!filename)
          ret = CURLE_OUT_OF_MEMORY;
      }
      if(!ret)
        ret = Curl_mime_add_header(&part->curlheaders,
                                   "Content-Disposition: %s%s%s%s%s%s%s",
                                   disposition,
                                   name? "; name=\"": "",
                                   name? name: "",
                                   name? "\"": "",
                                   filename? "; filename=\"": "",
                                   filename? filename: "",
                                   filename? "\"": "");
      Curl_safefree(name);
      Curl_safefree(filename);
      if(ret)
        return ret;
      }
    }

  /* Issue Content-Type header. */
  if(contenttype) {
    ret = add_content_type(&part->curlheaders, contenttype, boundary);
    if(ret)
      return ret;
  }

  /* Content-Transfer-Encoding header. */
  if(!search_header(part->userheaders, "Content-Transfer-Encoding")) {
    if(part->encoder)
      cte = part->encoder->name;
    else if(contenttype && strategy == MIMESTRATEGY_MAIL &&
     part->kind != MIMEKIND_MULTIPART)
      cte = "8bit";
    if(cte) {
      ret = Curl_mime_add_header(&part->curlheaders,
                                 "Content-Transfer-Encoding: %s", cte);
      if(ret)
        return ret;
    }
  }

  /* If we were reading curl-generated headers, restart with new ones (this
     should not occur). */
  if(part->state.state == MIMESTATE_CURLHEADERS)
    mimesetstate(&part->state, MIMESTATE_CURLHEADERS, part->curlheaders);

  /* Process subparts. */
  if(part->kind == MIMEKIND_MULTIPART && mime) {
    curl_mimepart *subpart;

    disposition = NULL;
    if(content_type_match(contenttype, "multipart/form-data"))
      disposition = "form-data";
    for(subpart = mime->firstpart; subpart; subpart = subpart->nextpart) {
      ret = Curl_mime_prepare_headers(subpart, NULL, disposition, strategy);
      if(ret)
        return ret;
    }
  }


// Source: mime.c
// Lines 1799-1933
