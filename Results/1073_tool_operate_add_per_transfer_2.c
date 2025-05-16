static CURLcode add_per_transfer(struct per_transfer **per)
{
  struct per_transfer *p;
  p = calloc(sizeof(struct per_transfer), 1);
  if(!p)
    return CURLE_OUT_OF_MEMORY;
  if(!transfers)
    /* first entry */
    transfersl = transfers = p;
  else {
    /* make the last node point to the new node */
    transfersl->next = p;
    /* make the new node point back to the formerly last node */
    p->prev = transfersl;
    /* move the last node pointer to the new entry */
    transfersl = p;
  }
  *per = p;
  all_xfers++; /* count total number of transfers added */
  return CURLE_OK;
}


// Source: tool_operate.c
// Lines 211-231
