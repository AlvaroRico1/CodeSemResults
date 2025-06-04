create_tmp_var_name (const char *prefix)
{
  char *tmp_name;

  if (prefix)
    {
      char *preftmp = ASTRDUP (prefix);

      remove_suffix (preftmp, strlen (preftmp));
      clean_symbol_name (preftmp);

      prefix = preftmp;
    }


// Source: gimple-expr.c
// Lines 418-430
