bitmap_tree_rotate_right (bitmap_element * &t)
{
  bitmap_element *e = t->prev;
  t->prev = t->prev->next;
  e->next = t;
  t = e;
}


// Source: bitmap.c
// Lines 453-459
