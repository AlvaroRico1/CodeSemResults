  brig_string_slot s_slot;
  brig_string_slot **slot;
  char *str2;

  str2 = xstrdup (str);

  if (sanitize)
    hsa_sanitize_name (str2);
  s_slot.s = str2;
  s_slot.len = slen;
  s_slot.prefix = prefix;
  s_slot.offset = 0;

  slot = brig_string_htab->find_slot (&s_slot, INSERT);
  if (*slot == NULL)
    {
      brig_string_slot *new_slot = XCNEW (brig_string_slot);

      /* In theory we should fill in BrigData but that would mean copying
	 the string to a buffer for no reason, so we just emulate it.  */
      offset = brig_data.add (&hdr_len, sizeof (hdr_len));
      if (prefix)
	brig_data.add (&prefix, 1);

      brig_data.add (str2, slen);
      brig_data.round_size_up (4);

      /* TODO: could use the string we just copied into
	 brig_string->cur_chunk */
      new_slot->s = str2;
      new_slot->len = slen;
      new_slot->prefix = prefix;
      new_slot->offset = offset;
      *slot = new_slot;
    }
  else
    {
      offset = (*slot)->offset;
      free (str2);
    }

  return offset;
}


// Source: hsa-brig.c
// Lines 406-448
