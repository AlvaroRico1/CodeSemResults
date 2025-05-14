// Source: curl/lib/mime.c
// Lines 1005-1007
static size_t mime_subparts_read(char *buffer, size_t size, size_t nitems,
                                 void *instream, bool *hasread)
{
  curl_mime *mime = (curl_mime *) instream;
  size_t cursize = 0;
#ifdef CURL_DOES_CONVERSIONS
  char *convbuf = buffer;
#endif

  (void) size;   /* Always 1. */

  while(nitems) {
    size_t sz = 0;
    curl_mimepart *part = mime->state.ptr;
    switch(mime->state.state) {
    case MIMESTATE_BEGIN:
    case MIMESTATE_BODY:
#ifdef CURL_DOES_CONVERSIONS
      convbuf = buffer;
#endif
      mimesetstate(&mime->state, MIMESTATE_BOUNDARY1, mime->firstpart);
      /* The first boundary always follows the header termination empty line,
         so is always preceded by a CRLF. We can then spare 2 characters
         by skipping the leading CRLF in boundary. */
      mime->state.offset += 2;
      break;
    case MIMESTATE_BOUNDARY1:
      sz = readback_bytes(&mime->state, buffer, nitems, "\r\n--", 4, "");
      if(!sz)
        mimesetstate(&mime->state, MIMESTATE_BOUNDARY2, part);
      break;
    case MIMESTATE_BOUNDARY2:
      sz = readback_bytes(&mime->state, buffer, nitems, mime->boundary,
                          strlen(mime->boundary), part? "\r\n": "--\r\n");
      if(!sz) {
#ifdef CURL_DOES_CONVERSIONS
        if(mime->easy && convbuf < buffer) {
          CURLcode result = Curl_convert_to_network(mime->easy, convbuf,
                                                    buffer - convbuf);
          if(result)
            return READ_ERROR;
          convbuf = buffer;
        }
#endif
        mimesetstate(&mime->state, MIMESTATE_CONTENT, part);
      }
      break;
    case MIMESTATE_CONTENT:
      if(!part) {
        mimesetstate(&mime->state, MIMESTATE_END, NULL);
        break;
      }
      sz = readback_part(part, buffer, nitems, hasread);
      switch(sz) {
      case CURL_READFUNC_ABORT:
      case CURL_READFUNC_PAUSE:
      case READ_ERROR:
      case STOP_FILLING:
        return cursize? cursize: sz;
      case 0:
#ifdef CURL_DOES_CONVERSIONS
        convbuf = buffer;
#endif
        mimesetstate(&mime->state, MIMESTATE_BOUNDARY1, part->nextpart);
        break;
      }
      break;
    case MIMESTATE_END:
      return cursize;
    default:
      break;    /* other values not used in mime state. */
    }

    /* Bump buffer and counters according to read size. */
    cursize += sz;
    buffer += sz;
    nitems -= sz;
  }

#ifdef CURL_DOES_CONVERSIONS
      if(mime->easy && convbuf < buffer &&
         mime->state.state <= MIMESTATE_CONTENT) {
        CURLcode result = Curl_convert_to_network(mime->easy, convbuf,
                                                  buffer - convbuf);
        if(result)
          return READ_ERROR;
      }
#endif

  return cursize;
}
