bitmap_tree_view (bitmap head)
{
  bitmap_element *ptr;

  gcc_assert (! head->tree_form);

  ptr = head->first;
  while (ptr)
    {
      ptr->prev = NULL;
      ptr = ptr->next;
    }

  head->tree_form = true;
}


// Source: bitmap.c
// Lines 689-703
