bitmap_list_view (bitmap head)
{
  bitmap_element *ptr;

  gcc_assert (head->tree_form);

  ptr = head->first;
  if (ptr)
    {
      while (ptr->prev)
	bitmap_tree_rotate_right (ptr);
      head->first = ptr;
      head->first = bitmap_tree_listify_from (head, ptr);
    }

  head->tree_form = false;
}


// Source: bitmap.c
// Lines 665-681
