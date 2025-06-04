static CURLcode tftp_parse_option_ack(struct tftp_state_data *state,
                                      const char *ptr, int len)
{
  const char *tmp = ptr;
  struct Curl_easy *data = state->data;

  /* if OACK doesn't contain blksize option, the default (512) must be used */
  state->blksize = TFTP_BLKSIZE_DEFAULT;

  while(tmp < ptr + len) {
    const char *option, *value;

    tmp = tftp_option_get(tmp, ptr + len - tmp, &option, &value);
    if(!tmp) {
      failf(data, "Malformed ACK packet, rejecting");
      return CURLE_TFTP_ILLEGAL;
    }

    infof(data, "got option=(%s) value=(%s)", option, value);

    if(checkprefix(option, TFTP_OPTION_BLKSIZE)) {
      long blksize;

      blksize = strtol(value, NULL, 10);

      if(!blksize) {
        failf(data, "invalid blocksize value in OACK packet");
        return CURLE_TFTP_ILLEGAL;
      }
      if(blksize > TFTP_BLKSIZE_MAX) {
        failf(data, "%s (%d)", "blksize is larger than max supported",
              TFTP_BLKSIZE_MAX);
        return CURLE_TFTP_ILLEGAL;
      }
      else if(blksize < TFTP_BLKSIZE_MIN) {
        failf(data, "%s (%d)", "blksize is smaller than min supported",
              TFTP_BLKSIZE_MIN);
        return CURLE_TFTP_ILLEGAL;
      }
      else if(blksize > state->requested_blksize) {
        /* could realloc pkt buffers here, but the spec doesn't call out
         * support for the server requesting a bigger blksize than the client
         * requests */
        failf(data, "%s (%ld)",
              "server requested blksize larger than allocated", blksize);
        return CURLE_TFTP_ILLEGAL;
      }

      state->blksize = (int)blksize;
      infof(data, "%s (%d) %s (%d)", "blksize parsed from OACK",
            state->blksize, "requested", state->requested_blksize);
    }
    else if(checkprefix(option, TFTP_OPTION_TSIZE)) {
      long tsize = 0;

      tsize = strtol(value, NULL, 10);
      infof(data, "%s (%ld)", "tsize parsed from OACK", tsize);

      /* tsize should be ignored on upload: Who cares about the size of the
         remote file? */
      if(!data->set.upload) {
        if(!tsize) {
          failf(data, "invalid tsize -:%s:- value in OACK packet", value);
          return CURLE_TFTP_ILLEGAL;
        }
        Curl_pgrsSetDownloadSize(data, tsize);
      }
    }
  }

  return CURLE_OK;
}


// Source: tftp.c
// Lines 310-381
