// Source: curl/lib/content_encoding.c
// Lines 263-265
static CURLcode deflate_init_writer(struct Curl_easy *data,
                                    struct contenc_writer *writer)
{
  struct zlib_params *zp = (struct zlib_params *) &writer->params;
  z_stream *z = &zp->z;     /* zlib state structure */

  if(!writer->downstream)
    return CURLE_WRITE_ERROR;

  /* Initialize zlib */
  z->zalloc = (alloc_func) zalloc_cb;
  z->zfree = (free_func) zfree_cb;

  if(inflateInit(z) != Z_OK)
    return process_zlib_error(data, z);
  zp->zlib_init = ZLIB_INIT;
  return CURLE_OK;
}
