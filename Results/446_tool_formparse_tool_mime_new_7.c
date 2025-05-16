static struct tool_mime *tool_mime_new(struct tool_mime *parent,
                                       toolmimekind kind)
{
  struct tool_mime *m = (struct tool_mime *) calloc(1, sizeof(*m));

  if(m) {
    m->kind = kind;
    m->parent = parent;
    if(parent) {
      m->prev = parent->subparts;
      parent->subparts = m;
    }
  }
  return m;
}


// Source: tool_formparse.c
// Lines 45-59
