static bool auth_digest_get_key_value(const char *chlg,
                                      const char *key,
                                      char *value,
                                      size_t max_val_len,
                                      char end_char)
{
  char *find_pos;
  size_t i;

  find_pos = strstr(chlg, key);
  if(!find_pos)
    return FALSE;

  find_pos += strlen(key);

  for(i = 0; *find_pos && *find_pos != end_char && i < max_val_len - 1; ++i)
    value[i] = *find_pos++;
  value[i] = '\0';

  return TRUE;
}


// Source: digest.c
// Lines 195-215
