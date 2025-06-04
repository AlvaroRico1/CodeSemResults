static int addbyter(int output, FILE *data)
{
  struct nsprintf *infop = (struct nsprintf *)data;
  unsigned char outc = (unsigned char)output;

  if(infop->length < infop->max) {
    /* only do this if we haven't reached max length yet */
    infop->buffer[0] = outc; /* store */
    infop->buffer++; /* increase pointer */
    infop->length++; /* we are now one byte larger */
    return outc;     /* fputc() returns like this on success */
  }
  return -1;
}


// Source: mprintf.c
// Lines 992-1005
