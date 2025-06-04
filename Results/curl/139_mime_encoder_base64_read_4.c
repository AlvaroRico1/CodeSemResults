static size_t encoder_base64_read(char *buffer, size_t size, bool ateof,
                                curl_mimepart *part)
{
  struct mime_encoder_state *st = &part->encstate;
  size_t cursize = 0;
  int i;
  char *ptr = buffer;

  while(st->bufbeg < st->bufend) {
    /* Line full ? */
    if(st->pos > MAX_ENCODED_LINE_LENGTH - 4) {
      /* Yes, we need 2 characters for CRLF. */
      if(size < 2) {
        if(!cursize)
          return STOP_FILLING;
        break;
      }
      *ptr++ = '\r';
      *ptr++ = '\n';
      st->pos = 0;
      cursize += 2;
      size -= 2;
    }

    /* Be sure there is enough space and input data for a base64 group. */
    if(size < 4) {
      if(!cursize)
        return STOP_FILLING;
      break;
    }
    if(st->bufend - st->bufbeg < 3)
      break;

    /* Encode three bytes as four characters. */
    i = st->buf[st->bufbeg++] & 0xFF;
    i = (i << 8) | (st->buf[st->bufbeg++] & 0xFF);
    i = (i << 8) | (st->buf[st->bufbeg++] & 0xFF);
    *ptr++ = base64[(i >> 18) & 0x3F];
    *ptr++ = base64[(i >> 12) & 0x3F];
    *ptr++ = base64[(i >> 6) & 0x3F];
    *ptr++ = base64[i & 0x3F];
    cursize += 4;
    st->pos += 4;
    size -= 4;
  }

  /* If at eof, we have to flush the buffered data. */
  if(ateof) {
    if(size < 4) {
      if(!cursize)
        return STOP_FILLING;
    }
    else {
      /* Buffered data size can only be 0, 1 or 2. */
      ptr[2] = ptr[3] = '=';
      i = 0;
      switch(st->bufend - st->bufbeg) {
      case 2:
        i = (st->buf[st->bufbeg + 1] & 0xFF) << 8;
        /* FALLTHROUGH */
      case 1:
        i |= (st->buf[st->bufbeg] & 0xFF) << 16;
        ptr[0] = base64[(i >> 18) & 0x3F];
        ptr[1] = base64[(i >> 12) & 0x3F];
        if(++st->bufbeg != st->bufend) {
          ptr[2] = base64[(i >> 6) & 0x3F];
          st->bufbeg++;
        }
        cursize += 4;
        st->pos += 4;
        break;
      }
    }
  }

#ifdef CURL_DOES_CONVERSIONS
  /* This is now textual data, Convert character codes. */
  if(part->easy && cursize) {
    CURLcode result = Curl_convert_to_network(part->easy, buffer, cursize);
    if(result)
      return READ_ERROR;
  }
#endif

  return cursize;
}


// Source: mime.c
// Lines 409-494
