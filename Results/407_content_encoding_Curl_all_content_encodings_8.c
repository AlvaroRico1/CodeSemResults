char *Curl_all_content_encodings(void)
{
  size_t len = 0;
  const struct content_encoding * const *cep;
  const struct content_encoding *ce;
  char *ace;

  for(cep = encodings; *cep; cep++) {
    ce = *cep;
    if(!strcasecompare(ce->name, CONTENT_ENCODING_DEFAULT))
      len += strlen(ce->name) + 2;
  }

  if(!len)
    return strdup(CONTENT_ENCODING_DEFAULT);

  ace = malloc(len);
  if(ace) {
    char *p = ace;
    for(cep = encodings; *cep; cep++) {
      ce = *cep;
      if(!strcasecompare(ce->name, CONTENT_ENCODING_DEFAULT)) {
        strcpy(p, ce->name);
        p += strlen(p);
        *p++ = ',';
        *p++ = ' ';
      }
    }
    p[-2] = '\0';
  }

  return ace;
}


// Source: content_encoding.c
// Lines 851-883
