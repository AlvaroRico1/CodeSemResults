remove_from_table (struct table_elt *elt, unsigned int hash)
{
  if (elt == 0)
    return;

  /* Mark this element as removed.  See cse_insn.  */
  elt->first_same_value = 0;

  /* Remove the table element from its equivalence class.  */

  {
    struct table_elt *prev = elt->prev_same_value;
    struct table_elt *next = elt->next_same_value;

    if (next)
      next->prev_same_value = prev;

    if (prev)
      prev->next_same_value = next;
    else
      {
	struct table_elt *newfirst = next;
	while (next)
	  {
	    next->first_same_value = newfirst;
	    next = next->next_same_value;
	  }
      }


// Source: cse.c
// Lines 1364-1391
