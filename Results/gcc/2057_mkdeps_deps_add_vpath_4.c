deps_add_vpath (class mkdeps *d, const char *vpath)
{
  const char *elem, *p;

  for (elem = vpath; *elem; elem = p)
    {
      for (p = elem; *p && *p != ':'; p++)
	continue;
      mkdeps::velt elt;
      elt.len = p - elem;
      char *str = XNEWVEC (char, elt.len + 1);
      elt.str = str;
      memcpy (str, elem, elt.len);
      str[elt.len] = '\0';
      if (*p == ':')
	p++;

      d->vpath.push (elt);
    }


// Source: mkdeps.c
// Lines 303-321
