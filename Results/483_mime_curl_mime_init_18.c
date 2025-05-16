curl_mime *curl_mime_init(struct Curl_easy *easy)
{
  curl_mime *mime;

  mime = (curl_mime *) malloc(sizeof(*mime));

  if(mime) {
    mime->easy = easy;
    mime->parent = NULL;
    mime->firstpart = NULL;
    mime->lastpart = NULL;

    memset(mime->boundary, '-', 24);
    if(Curl_rand_hex(easy, (unsigned char *) &mime->boundary[24],
                     MIME_RAND_BOUNDARY_CHARS + 1)) {
      /* failed to get random separator, bail out */
      free(mime);
      return NULL;
    }
    mimesetstate(&mime->state, MIMESTATE_BEGIN, NULL);
  }

  return mime;
}


// Source: mime.c
// Lines 1307-1330
