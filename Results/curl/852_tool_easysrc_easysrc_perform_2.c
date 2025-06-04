CURLcode easysrc_perform(void)
{
  /* Note any setopt calls which we could not convert */
  if(easysrc_toohard) {
    int i;
    struct curl_slist *ptr;
    const char *c;
    CHKRET(easysrc_add(&easysrc_code, ""));
    /* Preamble comment */
    for(i = 0; ((c = srchard[i]) != NULL); i++)
      CHKRET(easysrc_add(&easysrc_code, c));
    /* Each unconverted option */
    if(easysrc_toohard) {
      for(ptr = easysrc_toohard->first; ptr; ptr = ptr->next)
        CHKRET(easysrc_add(&easysrc_code, ptr->data));
    }
    CHKRET(easysrc_add(&easysrc_code, ""));
    CHKRET(easysrc_add(&easysrc_code, "*/"));

    slist_wc_free_all(easysrc_toohard);
    easysrc_toohard = NULL;
  }


// Source: tool_easysrc.c
// Lines 135-156
