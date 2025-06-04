static int storebuffer(int output, FILE *data)
{
  char **buffer = (char **)data;
  unsigned char outc = (unsigned char)output;
  **buffer = outc;
  (*buffer)++;
  return outc; /* act like fputc() ! */
}


// Source: mprintf.c
// Lines 1102-1109
