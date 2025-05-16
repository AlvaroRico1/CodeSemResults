static size_t cookie_hash_domain(const char *domain, const size_t len)
{
  const char *end = domain + len;
  size_t h = 5381;

  while(domain < end) {
    h += h << 5;
    h ^= Curl_raw_toupper(*domain++);
  }

  return (h % COOKIE_HASH_SIZE);
}


// Source: cookie.c
// Lines 255-266
