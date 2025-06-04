static char *unescape_word(struct Curl_easy *data, const char *inputbuff)
{
  char *newp = NULL;
  char *dictp;
  size_t len;

  CURLcode result = Curl_urldecode(data, inputbuff, 0, &newp, &len,
                                   REJECT_NADA);
  if(!newp || result)
    return NULL;

  dictp = malloc(len*2 + 1); /* add one for terminating zero */
  if(dictp) {
    char *ptr;
    char ch;
    int olen = 0;
    /* According to RFC2229 section 2.2, these letters need to be escaped with
       \[letter] */
    for(ptr = newp;
        (ch = *ptr) != 0;
        ptr++) {
      if((ch <= 32) || (ch == 127) ||
          (ch == '\'') || (ch == '\"') || (ch == '\\')) {
        dictp[olen++] = '\\';
      }
      dictp[olen++] = ch;
    }
    dictp[olen] = 0;
  }


// Source: dict.c
// Lines 99-127
