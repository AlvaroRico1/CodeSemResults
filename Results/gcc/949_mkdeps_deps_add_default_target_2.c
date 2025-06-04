deps_add_default_target (class mkdeps *d, const char *tgt)
{
  /* Only if we have no targets.  */
  if (d->targets.size ())
    return;

  if (tgt[0] == '\0')
    d->targets.push (xstrdup ("-"));
  else
    {
#ifndef TARGET_OBJECT_SUFFIX
# define TARGET_OBJECT_SUFFIX ".o"
#endif
      const char *start = lbasename (tgt);
      char *o = (char *) alloca (strlen (start)
                                 + strlen (TARGET_OBJECT_SUFFIX) + 1);
      char *suffix;

      strcpy (o, start);

      suffix = strrchr (o, '.');
      if (!suffix)
        suffix = o + strlen (o);
      strcpy (suffix, TARGET_OBJECT_SUFFIX);

      deps_add_target (d, o, 1);
    }


// Source: mkdeps.c
// Lines 263-289
