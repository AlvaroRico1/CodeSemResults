add_partition_graph_edge (struct graph *pg, int i, int j, vec<ddr_p> *ddrs)
{
  struct graph_edge *e = add_edge (pg, i, j);

  /* If the edge is attached with data dependence relations, it means this
     dependence edge can be resolved by runtime alias checks.  */
  if (ddrs != NULL)
    {
      struct pg_edata *data = new pg_edata;

      gcc_assert (ddrs->length () > 0);
      e->data = data;
      data->alias_ddrs = vNULL;
      data->alias_ddrs.safe_splice (*ddrs);
    }


// Source: tree-loop-distribution.c
// Lines 2179-2193
