static struct tool_mime *tool_mime_new_data(struct tool_mime *parent,
                                            const char *data)
{
  struct tool_mime *m = NULL;

  data = strdup(data);
  if(data) {
    m = tool_mime_new(parent, TOOLMIME_DATA);
    if(!m)
      CONST_FREE(data);
    else
      m->data = data;
  }
  return m;
}


// Source: tool_formparse.c
// Lines 66-80
