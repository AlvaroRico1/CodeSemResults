parse_gcc_colors (void)
{
  const char *p, *q, *name, *val;
  char *b;
  size_t name_len = 0, val_len = 0;

  p = getenv ("GCC_COLORS"); /* Plural! */
  if (p == NULL)
    return true;
  if (*p == '\0')
    return false;

  name = q = p;
  val = NULL;
  /* From now on, be well-formed or you're gone.  */
  for (;;)
    if (*q == ':' || *q == '\0')
      {
	struct color_cap *cap;

	if (val)
	  val_len = q - val;
	else
	  name_len = q - name;
	/* Empty name without val (empty cap)
	   won't match and will be ignored.  */
	for (cap = color_dict; cap->name; cap++)
	  if (cap->name_len == name_len
	      && memcmp (cap->name, name, name_len) == 0)
	    break;
	/* If name unknown, go on for forward compatibility.  */
	if (cap->val && val)
	  {
	    if (cap->free_val)
	      free (CONST_CAST (char *, cap->val));
	    b = XNEWVEC (char, val_len + sizeof (SGR_SEQ ("")));
	    memcpy (b, SGR_START, strlen (SGR_START));
	    memcpy (b + strlen (SGR_START), val, val_len);
	    memcpy (b + strlen (SGR_START) + val_len, SGR_END,
		    sizeof (SGR_END));
	    cap->val = (const char *) b;
	    cap->free_val = true;
	  }
	if (*q == '\0')
	  return true;
	name = ++q;
	val = NULL;
      }
    else if (*q == '=')
      {
	if (q == name || val)
	  return true;

	name_len = q - name;
	val = ++q; /* Can be the empty string.  */
      }
    else if (val == NULL)
      q++; /* Accumulate name.  */
    else if (*q == ';' || (*q >= '0' && *q <= '9'))
      q++; /* Accumulate val.  Protect the terminal from being sent
	      garbage.  */
    else
      return true;
}


// Source: diagnostic-color.c
// Lines 136-199
