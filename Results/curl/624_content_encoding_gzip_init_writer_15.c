static CURLcode gzip_init_writer(struct Curl_easy *data,
                                 struct contenc_writer *writer)
{
  struct zlib_params *zp = (struct zlib_params *) &writer->params;
  z_stream *z = &zp->z;     /* zlib state structure */

  if(!writer->downstream)
    return CURLE_WRITE_ERROR;

  /* Initialize zlib */
  z->zalloc = (alloc_func) zalloc_cb;
  z->zfree = (free_func) zfree_cb;

  if(strcmp(zlibVersion(), "1.2.0.4") >= 0) {
    /* zlib ver. >= 1.2.0.4 supports transparent gzip decompressing */
    if(inflateInit2(z, MAX_WBITS + 32) != Z_OK) {
      return process_zlib_error(data, z);
    }
    zp->zlib_init = ZLIB_INIT_GZIP; /* Transparent gzip decompress state */
  }
  else {
    /* we must parse the gzip header and trailer ourselves */
    if(inflateInit2(z, -MAX_WBITS) != Z_OK) {
      return process_zlib_error(data, z);
    }
    zp->trailerlen = 8; /* A CRC-32 and a 32-bit input size (RFC 1952, 2.2) */
    zp->zlib_init = ZLIB_INIT; /* Initial call state */
  }

  return CURLE_OK;
}


// Source: content_encoding.c
// Lines 319-349
