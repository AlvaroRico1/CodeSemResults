CURLcode Curl_pop3_write(struct Curl_easy *data, char *str, size_t nread)
{
  /* This code could be made into a special function in the handler struct */
  CURLcode result = CURLE_OK;
  struct SingleRequest *k = &data->req;
  struct connectdata *conn = data->conn;
  struct pop3_conn *pop3c = &conn->proto.pop3c;
  bool strip_dot = FALSE;
  size_t last = 0;
  size_t i;

  /* Search through the buffer looking for the end-of-body marker which is
     5 bytes (0d 0a 2e 0d 0a). Note that a line starting with a dot matches
     the eob so the server will have prefixed it with an extra dot which we
     need to strip out. Additionally the marker could of course be spread out
     over 5 different data chunks. */
  for(i = 0; i < nread; i++) {
    size_t prev = pop3c->eob;

    switch(str[i]) {
    case 0x0d:
      if(pop3c->eob == 0) {
        pop3c->eob++;

        if(i) {
          /* Write out the body part that didn't match */
          result = Curl_client_write(data, CLIENTWRITE_BODY, &str[last],
                                     i - last);

          if(result)
            return result;

          last = i;
        }
      }
      else if(pop3c->eob == 3)
        pop3c->eob++;
      else
        /* If the character match wasn't at position 0 or 3 then restart the
           pattern matching */
        pop3c->eob = 1;
      break;

    case 0x0a:
      if(pop3c->eob == 1 || pop3c->eob == 4)
        pop3c->eob++;
      else
        /* If the character match wasn't at position 1 or 4 then start the
           search again */
        pop3c->eob = 0;
      break;

    case 0x2e:
      if(pop3c->eob == 2)
        pop3c->eob++;
      else if(pop3c->eob == 3) {
        /* We have an extra dot after the CRLF which we need to strip off */
        strip_dot = TRUE;
        pop3c->eob = 0;
      }
      else
        /* If the character match wasn't at position 2 then start the search
           again */
        pop3c->eob = 0;
      break;

    default:
      pop3c->eob = 0;
      break;
    }

    /* Did we have a partial match which has subsequently failed? */
    if(prev && prev >= pop3c->eob) {
      /* Strip can only be non-zero for the very first mismatch after CRLF
         and then both prev and strip are equal and nothing will be output
         below */
      while(prev && pop3c->strip) {
        prev--;
        pop3c->strip--;
      }

      if(prev) {
        /* If the partial match was the CRLF and dot then only write the CRLF
           as the server would have inserted the dot */
        if(strip_dot && prev - 1 > 0) {
          result = Curl_client_write(data, CLIENTWRITE_BODY, (char *)POP3_EOB,
                                     prev - 1);
        }
        else if(!strip_dot) {
          result = Curl_client_write(data, CLIENTWRITE_BODY, (char *)POP3_EOB,
                                     prev);
        }
        else {
          result = CURLE_OK;
        }

        if(result)
          return result;

        last = i;
        strip_dot = FALSE;
      }
    }
  }

  if(pop3c->eob == POP3_EOB_LEN) {
    /* We have a full match so the transfer is done, however we must transfer
    the CRLF at the start of the EOB as this is considered to be part of the
    message as per RFC-1939, sect. 3 */
    result = Curl_client_write(data, CLIENTWRITE_BODY, (char *)POP3_EOB, 2);

    k->keepon &= ~KEEP_RECV;
    pop3c->eob = 0;

    return result;
  }

  if(pop3c->eob)
    /* While EOB is matching nothing should be output */
    return CURLE_OK;

  if(nread - last) {
    result = Curl_client_write(data, CLIENTWRITE_BODY, &str[last],
                               nread - last);
  }

  return result;
}


// Source: pop3.c
// Lines 1435-1562
