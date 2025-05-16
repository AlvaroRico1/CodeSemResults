static void mime_subparts_free(void *ptr)
{
  curl_mime *mime = (curl_mime *) ptr;

  if(mime && mime->parent) {
    mime->parent->freefunc = NULL;  /* Be sure we won't be called again. */
    cleanup_part_content(mime->parent);  /* Avoid dangling pointer in part. */
  }
  curl_mime_free(mime);
}


// Source: mime.c
// Lines 1174-1183
