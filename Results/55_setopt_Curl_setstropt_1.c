// Source: curl/lib/setopt.c
// Lines 57-65
CURLcode Curl_setstropt(char **charp, const char *s)
{
  /* Release the previous storage at `charp' and replace by a dynamic storage
     copy of `s'. Return CURLE_OK or CURLE_OUT_OF_MEMORY. */

  Curl_safefree(*charp);

  if(s) {
    char *str = strdup(s);

    if(str) {
      size_t len = strlen(str);
      if(len > CURL_MAX_INPUT_LENGTH) {
        free(str);
        return CURLE_BAD_FUNCTION_ARGUMENT;
      }
    }
    if(!str)
      return CURLE_OUT_OF_MEMORY;

    *charp = str;
  }
