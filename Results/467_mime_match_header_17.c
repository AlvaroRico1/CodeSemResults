static char *match_header(struct curl_slist *hdr, const char *lbl, size_t len)
{
  char *value = NULL;

  if(strncasecompare(hdr->data, lbl, len) && hdr->data[len] == ':')
    for(value = hdr->data + len + 1; *value == ' '; value++)
      ;
  return value;
}


// Source: mime.c
// Lines 308-316
