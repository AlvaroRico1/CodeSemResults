bitmap_tree_find_element (bitmap head, unsigned int indx)
{
  if (head->current == NULL
      || head->indx == indx)
    return head->current;

  /* Usage can be NULL due to allocated bitmaps for which we do not
     call initialize function.  */
  bitmap_usage *usage = NULL;
  if (GATHER_STATISTICS)
    usage = bitmap_mem_desc.get_descriptor_for_instance (head);

  /* This bitmap has more than one element, and we're going to look
     through the elements list.  Count that as a search.  */
  if (GATHER_STATISTICS && usage)
    usage->m_nsearches++;

  bitmap_element *element = bitmap_tree_splay (head, head->first, indx);
  gcc_checking_assert (element != NULL);
  head->first = element;
  head->current = element;
  head->indx = element->indx;
  if (element->indx != indx)
    element = 0;
  return element;
}


// Source: bitmap.c
// Lines 563-588
