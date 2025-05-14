// Source: curl/lib/http_chunks.c
// Lines 113-122
CHUNKcode Curl_httpchunk_read(struct Curl_easy *data,
                              char *datap,
                              ssize_t datalen,
                              ssize_t *wrotep,
                              CURLcode *extrap)
{
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  struct Curl_chunker *ch = &conn->chunk;
  struct SingleRequest *k = &data->req;
  size_t piece;
  curl_off_t length = (curl_off_t)datalen;
  size_t *wrote = (size_t *)wrotep;

  *wrote = 0; /* nothing's written yet */

  /* the original data is written to the client, but we go on with the
     chunk read process, to properly calculate the content length*/
  if(data->set.http_te_skip && !k->ignorebody) {
    result = Curl_client_write(data, CLIENTWRITE_BODY, datap, datalen);
    if(result) {
      *extrap = result;
      return CHUNKE_PASSTHRU_ERROR;
    }
  }

  while(length) {
    switch(ch->state) {
    case CHUNK_HEX:
      if(isxdigit_ascii(*datap)) {
        if(ch->hexindex < CHUNK_MAXNUM_LEN) {
          ch->hexbuffer[ch->hexindex] = *datap;
          datap++;
          length--;
          ch->hexindex++;
        }
        else {
          return CHUNKE_TOO_LONG_HEX; /* longer hex than we support */
        }
      }
      else {
        char *endptr;
        if(0 == ch->hexindex)
          /* This is illegal data, we received junk where we expected
             a hexadecimal digit. */
          return CHUNKE_ILLEGAL_HEX;

        /* length and datap are unmodified */
        ch->hexbuffer[ch->hexindex] = 0;

        /* convert to host encoding before calling strtoul */
        result = Curl_convert_from_network(data, ch->hexbuffer, ch->hexindex);
        if(result) {
          /* Curl_convert_from_network calls failf if unsuccessful */
          /* Treat it as a bad hex character */
          return CHUNKE_ILLEGAL_HEX;
        }

        if(curlx_strtoofft(ch->hexbuffer, &endptr, 16, &ch->datasize))
          return CHUNKE_ILLEGAL_HEX;
        ch->state = CHUNK_LF; /* now wait for the CRLF */
      }
      break;

    case CHUNK_LF:
      /* waiting for the LF after a chunk size */
      if(*datap == 0x0a) {
        /* we're now expecting data to come, unless size was zero! */
        if(0 == ch->datasize) {
          ch->state = CHUNK_TRAILER; /* now check for trailers */
        }
        else
          ch->state = CHUNK_DATA;
      }

      datap++;
      length--;
      break;

    case CHUNK_DATA:
      /* We expect 'datasize' of data. We have 'length' right now, it can be
         more or less than 'datasize'. Get the smallest piece.
      */
      piece = curlx_sotouz((ch->datasize >= length)?length:ch->datasize);

      /* Write the data portion available */
      if(!data->set.http_te_skip && !k->ignorebody) {
        if(!data->set.http_ce_skip && k->writer_stack)
          result = Curl_unencode_write(data, k->writer_stack, datap, piece);
        else
          result = Curl_client_write(data, CLIENTWRITE_BODY, datap, piece);

        if(result) {
          *extrap = result;
          return CHUNKE_PASSTHRU_ERROR;
        }
      }

      *wrote += piece;
      ch->datasize -= piece; /* decrease amount left to expect */
      datap += piece;    /* move read pointer forward */
      length -= piece;   /* decrease space left in this round */

      if(0 == ch->datasize)
        /* end of data this round, we now expect a trailing CRLF */
        ch->state = CHUNK_POSTLF;
      break;

    case CHUNK_POSTLF:
      if(*datap == 0x0a) {
        /* The last one before we go back to hex state and start all over. */
        Curl_httpchunk_init(data); /* sets state back to CHUNK_HEX */
      }
      else if(*datap != 0x0d)
        return CHUNKE_BAD_CHUNK;
      datap++;
      length--;
      break;

    case CHUNK_TRAILER:
      if((*datap == 0x0d) || (*datap == 0x0a)) {
        char *tr = Curl_dyn_ptr(&conn->trailer);
        /* this is the end of a trailer, but if the trailer was zero bytes
           there was no trailer and we move on */

        if(tr) {
          size_t trlen;
          result = Curl_dyn_add(&conn->trailer, (char *)"\x0d\x0a");
          if(result)
            return CHUNKE_OUT_OF_MEMORY;

          tr = Curl_dyn_ptr(&conn->trailer);
          trlen = Curl_dyn_len(&conn->trailer);
          /* Convert to host encoding before calling Curl_client_write */
          result = Curl_convert_from_network(data, tr, trlen);
          if(result)
            /* Curl_convert_from_network calls failf if unsuccessful */
            /* Treat it as a bad chunk */
            return CHUNKE_BAD_CHUNK;

          if(!data->set.http_te_skip) {
            result = Curl_client_write(data, CLIENTWRITE_HEADER, tr, trlen);
            if(result) {
              *extrap = result;
              return CHUNKE_PASSTHRU_ERROR;
            }
          }
          Curl_dyn_reset(&conn->trailer);
          ch->state = CHUNK_TRAILER_CR;
          if(*datap == 0x0a)
            /* already on the LF */
            break;
        }
        else {
          /* no trailer, we're on the final CRLF pair */
          ch->state = CHUNK_TRAILER_POSTCR;
          break; /* don't advance the pointer */
        }
      }
      else {
        result = Curl_dyn_addn(&conn->trailer, datap, 1);
        if(result)
          return CHUNKE_OUT_OF_MEMORY;
      }
      datap++;
      length--;
      break;

    case CHUNK_TRAILER_CR:
      if(*datap == 0x0a) {
        ch->state = CHUNK_TRAILER_POSTCR;
        datap++;
        length--;
      }
      else
        return CHUNKE_BAD_CHUNK;
      break;

    case CHUNK_TRAILER_POSTCR:
      /* We enter this state when a CR should arrive so we expect to
         have to first pass a CR before we wait for LF */
      if((*datap != 0x0d) && (*datap != 0x0a)) {
        /* not a CR then it must be another header in the trailer */
        ch->state = CHUNK_TRAILER;
        break;
      }
      if(*datap == 0x0d) {
        /* skip if CR */
        datap++;
        length--;
      }
      /* now wait for the final LF */
      ch->state = CHUNK_STOP;
      break;

    case CHUNK_STOP:
      if(*datap == 0x0a) {
        length--;

        /* Record the length of any data left in the end of the buffer
           even if there's no more chunks to read */
        ch->datasize = curlx_sotouz(length);

        return CHUNKE_STOP; /* return stop */
      }
      else
        return CHUNKE_BAD_CHUNK;
    }
  }
  return CHUNKE_OK;
}
