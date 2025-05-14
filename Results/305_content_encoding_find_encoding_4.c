// Source: curl/lib/content_encoding.c
// Lines 1017-1020
static const struct content_encoding *find_encoding(const char *name,
                                                    size_t len)
{
  const struct content_encoding * const *cep;

  for(cep = encodings; *cep; cep++) {
    const struct content_encoding *ce = *cep;
    if((strncasecompare(name, ce->name, len) && !ce->name[len]) ||
       (ce->alias && strncasecompare(name, ce->alias, len) && !ce->alias[len]))
      return ce;
  }
