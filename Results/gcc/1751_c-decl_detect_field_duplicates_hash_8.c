detect_field_duplicates_hash (tree fieldlist,
			      hash_table<nofree_ptr_hash <tree_node> > *htab)
{
  tree x, y;
  tree_node **slot;

  for (x = fieldlist; x ; x = DECL_CHAIN (x))
    if ((y = DECL_NAME (x)) != NULL_TREE)
      {
	slot = htab->find_slot (y, INSERT);
	if (*slot)
	  {
	    error ("duplicate member %q+D", x);
	    DECL_NAME (x) = NULL_TREE;
	  }
	*slot = y;
      }
    else if (RECORD_OR_UNION_TYPE_P (TREE_TYPE (x)))
      {
	detect_field_duplicates_hash (TYPE_FIELDS (TREE_TYPE (x)), htab);

	/* When using -fplan9-extensions, an anonymous field whose
	   name is a typedef can duplicate a field name.  */
	if (flag_plan9_extensions
	    && TYPE_NAME (TREE_TYPE (x)) != NULL_TREE
	    && TREE_CODE (TYPE_NAME (TREE_TYPE (x))) == TYPE_DECL)
	  {
	    tree xn = DECL_NAME (TYPE_NAME (TREE_TYPE (x)));
	    slot = htab->find_slot (xn, INSERT);
	    if (*slot)
	      error ("duplicate member %q+D", TYPE_NAME (TREE_TYPE (x)));
	    *slot = xn;
	  }


// Source: c-decl.c
// Lines 8173-8205
