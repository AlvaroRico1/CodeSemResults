// Source: curl/lib/mprintf.c
// Lines 1042-1044
static int alloc_addbyter(int output, FILE *data)
{
  struct asprintf *infop = (struct asprintf *)data;
  unsigned char outc = (unsigned char)output;

  if(Curl_dyn_addn(infop->b, &outc, 1)) {
    infop->fail = 1;
    return -1; /* fail */
  }
  return outc; /* fputc() returns like this on success */
}
