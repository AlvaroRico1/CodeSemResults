static CURLcode hsts_load(struct hsts *h, const char *file)
{
  CURLcode result = CURLE_OK;
  char *line = NULL;
  FILE *fp;

  /* we need a private copy of the file name so that the hsts cache file
     name survives an easy handle reset */
  free(h->filename);
  h->filename = strdup(file);
  if(!h->filename)
    return CURLE_OUT_OF_MEMORY;

  fp = fopen(file, FOPEN_READTEXT);
  if(fp) {
    line = malloc(MAX_HSTS_LINE);
    if(!line)
      goto fail;
    while(Curl_get_line(line, MAX_HSTS_LINE, fp)) {
      char *lineptr = line;
      while(*lineptr && ISBLANK(*lineptr))
        lineptr++;
      if(*lineptr == '#')
        /* skip commented lines */
        continue;

      hsts_add(h, lineptr);
    }


// Source: hsts.c
// Lines 474-501
