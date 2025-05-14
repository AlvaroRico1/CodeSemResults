// Source: curl/lib/dynbuf.c
// Lines 65-98
static CURLcode dyn_nappend(struct dynbuf *s,
                            const unsigned char *mem, size_t len)
{
  size_t indx = s->leng;
  size_t a = s->allc;
  size_t fit = len + indx + 1; /* new string + old string + zero byte */

  /* try to detect if there's rubbish in the struct */
  DEBUGASSERT(s->init == DYNINIT);
  DEBUGASSERT(s->toobig);
  DEBUGASSERT(indx < s->toobig);
  DEBUGASSERT(!s->leng || s->bufr);

  if(fit > s->toobig) {
    Curl_dyn_free(s);
    return CURLE_OUT_OF_MEMORY;
  }
  else if(!a) {
    DEBUGASSERT(!indx);
    /* first invoke */
    if(fit < MIN_FIRST_ALLOC)
      a = MIN_FIRST_ALLOC;
    else
      a = fit;
  }
  else {
    while(a < fit)
      a *= 2;
  }

  if(a != s->allc) {
    /* this logic is not using Curl_saferealloc() to make the tool not have to
       include that as well when it uses this code */
    void *p = realloc(s->bufr, a);
    if(!p) {
      Curl_safefree(s->bufr);
      s->leng = s->allc = 0;
      return CURLE_OUT_OF_MEMORY;
    }
    s->bufr = p;
    s->allc = a;
  }
