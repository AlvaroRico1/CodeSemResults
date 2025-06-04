candidates_list_and_hint (const char *arg, char *&str,
			  const auto_vec <const char *> &candidates)
{
  size_t len = 0;
  int i;
  const char *candidate;
  char *p;

  FOR_EACH_VEC_ELT (candidates, i, candidate)
    len += strlen (candidate) + 1;

  str = p = XNEWVEC (char, len);
  FOR_EACH_VEC_ELT (candidates, i, candidate)
    {
      len = strlen (candidate);
      memcpy (p, candidate, len);
      p[len] = ' ';
      p += len + 1;
    }
  p[-1] = '\0';
  return find_closest_string (arg, &candidates);
}


// Source: opts-common.c
// Lines 1238-1259
