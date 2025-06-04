lex_identifier (cpp_reader *pfile, const uchar *cur)
{
  size_t len;
  uchar *out = pfile->out.cur;
  cpp_hashnode *result;

  do
    *out++ = *cur++;
  while (is_numchar (*cur));

  CUR (pfile->context) = cur;
  len = out - pfile->out.cur;
  result = CPP_HASHNODE (ht_lookup (pfile->hash_table, pfile->out.cur,
				    len, HT_ALLOC));
  pfile->out.cur = out;
  return result;
}


// Source: traditional.c
// Lines 256-272
