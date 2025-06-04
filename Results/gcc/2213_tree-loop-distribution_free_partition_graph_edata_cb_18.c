free_partition_graph_edata_cb (struct graph *, struct graph_edge *e, void *)
{
  if (e->data != NULL)
    {
      struct pg_edata *data = (struct pg_edata *)e->data;
      data->alias_ddrs.release ();
      delete data;
    }


// Source: tree-loop-distribution.c
// Lines 2209-2216
