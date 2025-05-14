// Source: curl/lib/vtls/vtls.c
// Lines 1340-1346
static size_t multissl_version(char *buffer, size_t size)
{
  static const struct Curl_ssl *selected;
  static char backends[200];
  static size_t backends_len;
  const struct Curl_ssl *current;

  current = Curl_ssl == &Curl_ssl_multi ? available_backends[0] : Curl_ssl;

  if(current != selected) {
    char *p = backends;
    char *end = backends + sizeof(backends);
    int i;

    selected = current;

    backends[0] = '\0';

    for(i = 0; available_backends[i]; ++i) {
      char vb[200];
      bool paren = (selected != available_backends[i]);

      if(available_backends[i]->version(vb, sizeof(vb))) {
        p += msnprintf(p, end - p, "%s%s%s%s", (p != backends ? " " : ""),
                       (paren ? "(" : ""), vb, (paren ? ")" : ""));
      }
    }

    backends_len = p - backends;
  }
