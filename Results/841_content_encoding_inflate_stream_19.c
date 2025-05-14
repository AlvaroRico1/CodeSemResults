// Source: curl/lib/content_encoding.c
// Lines 164-164
static CURLcode inflate_stream(struct Curl_easy *data,
                               struct contenc_writer *writer,
                               zlibInitState started)
{
  struct zlib_params *zp = (struct zlib_params *) &writer->params;
  z_stream *z = &zp->z;         /* zlib state structure */
  uInt nread = z->avail_in;
  Bytef *orig_in = z->next_in;
  bool done = FALSE;
  CURLcode result = CURLE_OK;   /* Curl_client_write status */
  char *decomp;                 /* Put the decompressed data here. */

  /* Check state. */
  if(zp->zlib_init != ZLIB_INIT &&
     zp->zlib_init != ZLIB_INFLATING &&
     zp->zlib_init != ZLIB_INIT_GZIP &&
     zp->zlib_init != ZLIB_GZIP_INFLATING)
    return exit_zlib(data, z, &zp->zlib_init, CURLE_WRITE_ERROR);

  /* Dynamically allocate a buffer for decompression because it's uncommonly
     large to hold on the stack */
  decomp = malloc(DSIZ);
  if(!decomp)
    return exit_zlib(data, z, &zp->zlib_init, CURLE_OUT_OF_MEMORY);

  /* because the buffer size is fixed, iteratively decompress and transfer to
     the client via downstream_write function. */
  while(!done) {
    int status;                   /* zlib status */
    done = TRUE;

    /* (re)set buffer for decompressed output for every iteration */
    z->next_out = (Bytef *) decomp;
    z->avail_out = DSIZ;

#ifdef Z_BLOCK
    /* Z_BLOCK is only available in zlib ver. >= 1.2.0.5 */
    status = inflate(z, Z_BLOCK);
#else
    /* fallback for zlib ver. < 1.2.0.5 */
    status = inflate(z, Z_SYNC_FLUSH);
#endif

    /* Flush output data if some. */
    if(z->avail_out != DSIZ) {
      if(status == Z_OK || status == Z_STREAM_END) {
        zp->zlib_init = started;      /* Data started. */
        result = Curl_unencode_write(data, writer->downstream, decomp,
                                     DSIZ - z->avail_out);
        if(result) {
          exit_zlib(data, z, &zp->zlib_init, result);
          break;
        }
      }
    }

    /* Dispatch by inflate() status. */
    switch(status) {
    case Z_OK:
      /* Always loop: there may be unflushed latched data in zlib state. */
      done = FALSE;
      break;
    case Z_BUF_ERROR:
      /* No more data to flush: just exit loop. */
      break;
    case Z_STREAM_END:
      result = process_trailer(data, zp);
      break;
    case Z_DATA_ERROR:
      /* some servers seem to not generate zlib headers, so this is an attempt
         to fix and continue anyway */
      if(zp->zlib_init == ZLIB_INIT) {
        /* Do not use inflateReset2(): only available since zlib 1.2.3.4. */
        (void) inflateEnd(z);     /* don't care about the return code */
        if(inflateInit2(z, -MAX_WBITS) == Z_OK) {
          z->next_in = orig_in;
          z->avail_in = nread;
          zp->zlib_init = ZLIB_INFLATING;
          zp->trailerlen = 4; /* Tolerate up to 4 unknown trailer bytes. */
          done = FALSE;
          break;
        }
        zp->zlib_init = ZLIB_UNINIT;    /* inflateEnd() already called. */
      }
      /* FALLTHROUGH */
    default:
      result = exit_zlib(data, z, &zp->zlib_init, process_zlib_error(data, z));
      break;
    }
  }
  free(decomp);

  /* We're about to leave this call so the `nread' data bytes won't be seen
     again. If we are in a state that would wrongly allow restart in raw mode
     at the next call, assume output has already started. */
  if(nread && zp->zlib_init == ZLIB_INIT)
    zp->zlib_init = started;      /* Cannot restart anymore. */

  return result;
}
