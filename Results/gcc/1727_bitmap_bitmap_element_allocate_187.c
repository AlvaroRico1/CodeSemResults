bitmap_element_allocate (bitmap head)
{
  bitmap_element *element;
  bitmap_obstack *bit_obstack = head->obstack;

  if (bit_obstack)
    {
      element = bit_obstack->elements;

      if (element)
	/* Use up the inner list first before looking at the next
	   element of the outer list.  */
	if (element->next)
	  {
	    bit_obstack->elements = element->next;
	    bit_obstack->elements->prev = element->prev;
	  }
	else
	  /*  Inner list was just a singleton.  */
	  bit_obstack->elements = element->prev;
      else
	element = XOBNEW (&bit_obstack->obstack, bitmap_element);
    }
  else
    {
      element = bitmap_ggc_free;
      if (element)
	/* Use up the inner list first before looking at the next
	   element of the outer list.  */
	if (element->next)
	  {
	    bitmap_ggc_free = element->next;
	    bitmap_ggc_free->prev = element->prev;
	  }
	else
	  /*  Inner list was just a singleton.  */
	  bitmap_ggc_free = element->prev;
      else
	element = ggc_alloc<bitmap_element> ();
    }

  if (GATHER_STATISTICS)
    register_overhead (head, sizeof (bitmap_element));

  memset (element->bits, 0, sizeof (element->bits));

  return element;
}


// Source: bitmap.c
// Lines 102-149
