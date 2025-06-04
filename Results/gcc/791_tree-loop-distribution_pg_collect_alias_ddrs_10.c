pg_collect_alias_ddrs (struct graph *g, struct graph_edge *e, void *data)
{
  int i, j, component;
  struct pg_edge_callback_data *cbdata;
  struct pg_edata *edata = (struct pg_edata *) e->data;

  /* If the edge doesn't have attached data dependence, it represents
     compilation time known dependences.  This type dependence cannot
     be resolved by runtime alias check.  */
  if (edata == NULL || edata->alias_ddrs.length () == 0)
    return;

  cbdata = (struct pg_edge_callback_data *) data;
  i = e->src;
  j = e->dest;
  component = cbdata->vertices_component[i];
  /* Vertices are topologically sorted according to compilation time
     known dependences, so we can break strong connected components
     by removing edges of the opposite direction, i.e, edges pointing
     from vertice with smaller post number to vertice with bigger post
     number.  */
  if (g->vertices[i].post < g->vertices[j].post
      /* We only need to remove edges connecting vertices in the same
	 strong connected component to break it.  */
      && component == cbdata->vertices_component[j]
      /* Check if we want to break the strong connected component or not.  */
      && !bitmap_bit_p (cbdata->sccs_to_merge, component))
    cbdata->alias_ddrs->safe_splice (edata->alias_ddrs);
}


// Source: tree-loop-distribution.c
// Lines 2368-2396
