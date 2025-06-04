bitmap_tree_rotate_left (bitmap_element * &t)
{
  bitmap_element *e = t->next;
  t->next = t->next->prev;
  e->prev = t;
  t = e;
}


// Source: bitmap.c
// Lines 444-450
