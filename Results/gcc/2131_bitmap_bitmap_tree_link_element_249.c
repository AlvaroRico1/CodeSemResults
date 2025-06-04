bitmap_tree_link_element (bitmap head, bitmap_element *e)
{
  if (head->first == NULL)
    e->prev = e->next = NULL;
  else
    {
      bitmap_element *t = bitmap_tree_splay (head, head->first, e->indx);
      if (e->indx < t->indx)
	{
	  e->prev = t->prev;
	  e->next = t;
	  t->prev = NULL;
	}
      else if (e->indx > t->indx)
	{
	  e->next = t->next;
	  e->prev = t;
	  t->next = NULL;
	}
      else
	gcc_unreachable ();
    }


// Source: bitmap.c
// Lines 509-530
