// Source: curl/lib/hash.c
// Lines 259-260
size_t Curl_hash_str(void *key, size_t key_length, size_t slots_num)
{
  const char *key_str = (const char *) key;
  const char *end = key_str + key_length;
  size_t h = 5381;

  while(key_str < end) {
    h += h << 5;
    h ^= *key_str++;
  }

  return (h % slots_num);
}
