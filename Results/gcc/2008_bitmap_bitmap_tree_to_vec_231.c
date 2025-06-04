bitmap_tree_to_vec (vec<bitmap_element *> &elts, const_bitmap head)
{
  gcc_checking_assert (head->tree_form);
  auto_vec<bitmap_element *, 32> stack;
  bitmap_element *e = head->first;
  while (true)
    {
      while (e != NULL)
	{
	  stack.safe_push (e);
	  e = e->prev;
	}
      if (stack.is_empty ())
	break;

      e = stack.pop ();
      elts.safe_push (e);
      e = e->next;
    }
}


// Source: bitmap.c
// Lines 2602-2621
