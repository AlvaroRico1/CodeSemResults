static int multissl_setup(const struct Curl_ssl *backend)
{
  const char *env;
  char *env_tmp;

  if(Curl_ssl != &Curl_ssl_multi)
    return 1;

  if(backend) {
    Curl_ssl = backend;
    return 0;
  }

  if(!available_backends[0])
    return 1;

  env = env_tmp = curl_getenv("CURL_SSL_BACKEND");
#ifdef CURL_DEFAULT_SSL_BACKEND
  if(!env)
    env = CURL_DEFAULT_SSL_BACKEND;
#endif
  if(env) {
    int i;
    for(i = 0; available_backends[i]; i++) {
      if(strcasecompare(env, available_backends[i]->info.name)) {
        Curl_ssl = available_backends[i];
        free(env_tmp);
        return 0;
      }
    }
  }

  /* Fall back to first available backend */
  Curl_ssl = available_backends[0];
  free(env_tmp);
  return 0;
}


// Source: vtls.c
// Lines 1380-1416
