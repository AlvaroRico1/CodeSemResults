static CURLcode gzip_unencode_write(struct Curl_easy *data,
                                    struct contenc_writer *writer,
                                    const char *buf, size_t nbytes)
{
  struct zlib_params *zp = (struct zlib_params *) &writer->params;
  z_stream *z = &zp->z;     /* zlib state structure */

  if(zp->zlib_init == ZLIB_INIT_GZIP) {
    /* Let zlib handle the gzip decompression entirely */
    z->next_in = (Bytef *) buf;
    z->avail_in = (uInt) nbytes;
    /* Now uncompress the data */
    return inflate_stream(data, writer, ZLIB_INIT_GZIP);
  }

#ifndef OLD_ZLIB_SUPPORT
  /* Support for old zlib versions is compiled away and we are running with
     an old version, so return an error. */
  return exit_zlib(data, z, &zp->zlib_init, CURLE_WRITE_ERROR);

#else
  /* This next mess is to get around the potential case where there isn't
   * enough data passed in to skip over the gzip header.  If that happens, we
   * malloc a block and copy what we have then wait for the next call.  If
   * there still isn't enough (this is definitely a worst-case scenario), we
   * make the block bigger, copy the next part in and keep waiting.
   *
   * This is only required with zlib versions < 1.2.0.4 as newer versions
   * can handle the gzip header themselves.
   */

  switch(zp->zlib_init) {
  /* Skip over gzip header? */
  case ZLIB_INIT:
  {
    /* Initial call state */
    ssize_t hlen;

    switch(check_gzip_header((unsigned char *) buf, nbytes, &hlen)) {
    case GZIP_OK:
      z->next_in = (Bytef *) buf + hlen;
      z->avail_in = (uInt) (nbytes - hlen);
      zp->zlib_init = ZLIB_GZIP_INFLATING; /* Inflating stream state */
      break;

    case GZIP_UNDERFLOW:
      /* We need more data so we can find the end of the gzip header.  It's
       * possible that the memory block we malloc here will never be freed if
       * the transfer abruptly aborts after this point.  Since it's unlikely
       * that circumstances will be right for this code path to be followed in
       * the first place, and it's even more unlikely for a transfer to fail
       * immediately afterwards, it should seldom be a problem.
       */
      z->avail_in = (uInt) nbytes;
      z->next_in = malloc(z->avail_in);
      if(!z->next_in) {
        return exit_zlib(data, z, &zp->zlib_init, CURLE_OUT_OF_MEMORY);
      }
      memcpy(z->next_in, buf, z->avail_in);
      zp->zlib_init = ZLIB_GZIP_HEADER;  /* Need more gzip header data state */
      /* We don't have any data to inflate yet */
      return CURLE_OK;

    case GZIP_BAD:
    default:
      return exit_zlib(data, z, &zp->zlib_init, process_zlib_error(data, z));
    }

  }
  break;

  case ZLIB_GZIP_HEADER:
  {
    /* Need more gzip header data state */
    ssize_t hlen;
    z->avail_in += (uInt) nbytes;
    z->next_in = Curl_saferealloc(z->next_in, z->avail_in);
    if(!z->next_in) {
      return exit_zlib(data, z, &zp->zlib_init, CURLE_OUT_OF_MEMORY);
    }
    /* Append the new block of data to the previous one */
    memcpy(z->next_in + z->avail_in - nbytes, buf, nbytes);

    switch(check_gzip_header(z->next_in, z->avail_in, &hlen)) {
    case GZIP_OK:
      /* This is the zlib stream data */
      free(z->next_in);
      /* Don't point into the malloced block since we just freed it */
      z->next_in = (Bytef *) buf + hlen + nbytes - z->avail_in;
      z->avail_in = (uInt) (z->avail_in - hlen);
      zp->zlib_init = ZLIB_GZIP_INFLATING;   /* Inflating stream state */
      break;

    case GZIP_UNDERFLOW:
      /* We still don't have any data to inflate! */
      return CURLE_OK;

    case GZIP_BAD:
    default:
      return exit_zlib(data, z, &zp->zlib_init, process_zlib_error(data, z));
    }

  }
  break;

  case ZLIB_EXTERNAL_TRAILER:
    z->next_in = (Bytef *) buf;
    z->avail_in = (uInt) nbytes;
    return process_trailer(data, zp);

  case ZLIB_GZIP_INFLATING:
  default:
    /* Inflating stream state */
    z->next_in = (Bytef *) buf;
    z->avail_in = (uInt) nbytes;
    break;
  }

  if(z->avail_in == 0) {
    /* We don't have any data to inflate; wait until next time */
    return CURLE_OK;
  }

  /* We've parsed the header, now uncompress the data */
  return inflate_stream(data, writer, ZLIB_GZIP_INFLATING);
#endif
}


// Source: content_encoding.c
// Lines 435-561
