static void deflate_close_writer(struct Curl_easy *data,
                                 struct contenc_writer *writer)
{
  struct zlib_params *zp = (struct zlib_params *) &writer->params;
  z_stream *z = &zp->z;     /* zlib state structure */

  exit_zlib(data, z, &zp->zlib_init, CURLE_OK);
}


// Source: content_encoding.c
// Lines 299-306
