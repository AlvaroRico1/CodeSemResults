pg_skip_alias_edge (struct graph_edge *e)
{
  struct pg_edata *data = (struct pg_edata *)e->data;
  return (data != NULL && data->alias_ddrs.length () > 0);
}


// Source: tree-loop-distribution.c
// Lines 2200-2204
