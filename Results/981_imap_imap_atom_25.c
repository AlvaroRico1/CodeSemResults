// Source: curl/lib/imap.c
// Lines 1790-1811
static char *imap_atom(const char *str, bool escape_only)
{
  /* !checksrc! disable PARENBRACE 1 */
  const char atom_specials[] = "(){ %*]";
  const char *p1;
  char *p2;
  size_t backsp_count = 0;
  size_t quote_count = 0;
  bool others_exists = FALSE;
  size_t newlen = 0;
  char *newstr = NULL;

  if(!str)
    return NULL;

  /* Look for "atom-specials", counting the backslash and quote characters as
     these will need escaping */
  p1 = str;
  while(*p1) {
    if(*p1 == '\\')
      backsp_count++;
    else if(*p1 == '"')
      quote_count++;
    else if(!escape_only) {
      const char *p3 = atom_specials;

      while(*p3 && !others_exists) {
        if(*p1 == *p3)
          others_exists = TRUE;

        p3++;
      }
    }
