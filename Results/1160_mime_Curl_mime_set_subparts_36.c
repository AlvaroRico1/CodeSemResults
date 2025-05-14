// Source: curl/lib/mime.c
// Lines 1570-1570
CURLcode Curl_mime_set_subparts(curl_mimepart *part,
                                curl_mime *subparts, int take_ownership)
{
  curl_mime *root;

  if(!part)
    return CURLE_BAD_FUNCTION_ARGUMENT;

  /* Accept setting twice the same subparts. */
  if(part->kind == MIMEKIND_MULTIPART && part->arg == subparts)
    return CURLE_OK;

  cleanup_part_content(part);

  if(subparts) {
    /* Must belong to the same data handle. */
    if(part->easy && subparts->easy && part->easy != subparts->easy)
      return CURLE_BAD_FUNCTION_ARGUMENT;

    /* Should not have been attached already. */
    if(subparts->parent)
      return CURLE_BAD_FUNCTION_ARGUMENT;

    /* Should not be the part's root. */
    root = part->parent;
    if(root) {
      while(root->parent && root->parent->parent)
        root = root->parent->parent;
      if(subparts == root) {
        if(part->easy)
          failf(part->easy, "Can't add itself as a subpart!");
        return CURLE_BAD_FUNCTION_ARGUMENT;
      }
    }

    subparts->parent = part;
    /* Subparts are processed internally: no read callback. */
    part->seekfunc = mime_subparts_seek;
    part->freefunc = take_ownership? mime_subparts_free: mime_subparts_unbind;
    part->arg = subparts;
    part->datasize = -1;
    part->kind = MIMEKIND_MULTIPART;
  }

  return CURLE_OK;
}
