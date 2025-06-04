remove_attribute (const char *attr_name, tree list)
{
  tree *p;
  gcc_checking_assert (attr_name[0] != '_');

  for (p = &list; *p;)
    {
      tree l = *p;

      tree attr = get_attribute_name (l);
      if (is_attribute_p (attr_name, attr))
	*p = TREE_CHAIN (l);
      else
	p = &TREE_CHAIN (l);
    }

  return list;
}


// Source: attribs.c
// Lines 1407-1424
