ht_lookup_with_hash (cpp_hash_table *table, const unsigned char *str,
		     size_t len, unsigned int hash,
		     enum ht_lookup_option insert)
{
  unsigned int hash2;
  unsigned int index;
  unsigned int deleted_index = table->nslots;
  size_t sizemask;
  hashnode node;

  sizemask = table->nslots - 1;
  index = hash & sizemask;
  table->searches++;

  node = table->entries[index];

  if (node != NULL)
    {
      if (node == DELETED)
	deleted_index = index;
      else if (node->hash_value == hash
	       && HT_LEN (node) == (unsigned int) len
	       && !memcmp (HT_STR (node), str, len))
	return node;

      /* hash2 must be odd, so we're guaranteed to visit every possible
	 location in the table during rehashing.  */
      hash2 = ((hash * 17) & sizemask) | 1;

      for (;;)
	{
	  table->collisions++;
	  index = (index + hash2) & sizemask;
	  node = table->entries[index];
	  if (node == NULL)
	    break;

	  if (node == DELETED)
	    {
	      if (deleted_index != table->nslots)
		deleted_index = index;
	    }
	  else if (node->hash_value == hash
		   && HT_LEN (node) == (unsigned int) len
		   && !memcmp (HT_STR (node), str, len))
	    return node;
	}
    }

  if (insert == HT_NO_INSERT)
    return NULL;

  /* We prefer to overwrite the first deleted slot we saw.  */
  if (deleted_index != table->nslots)
    index = deleted_index;

  node = (*table->alloc_node) (table);
  table->entries[index] = node;

  HT_LEN (node) = (unsigned int) len;
  node->hash_value = hash;

  if (table->alloc_subobject)
    {
      char *chars = (char *) table->alloc_subobject (len + 1);
      memcpy (chars, str, len);
      chars[len] = '\0';
      HT_STR (node) = (const unsigned char *) chars;
    }


// Source: symtab.c
// Lines 99-167
