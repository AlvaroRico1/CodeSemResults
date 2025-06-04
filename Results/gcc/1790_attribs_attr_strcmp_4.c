attr_strcmp (const void *v1, const void *v2)
{
  const char *c1 = *(char *const*)v1;
  const char *c2 = *(char *const*)v2;
  return strcmp (c1, c2);
}


// Source: attribs.c
// Lines 878-883
