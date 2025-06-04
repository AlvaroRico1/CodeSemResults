free_rdg (struct graph *rdg)
{
  int i;

  for (i = 0; i < rdg->n_vertices; i++)
    {
      struct vertex *v = &(rdg->vertices[i]);
      struct graph_edge *e;

      for (e = v->succ; e; e = e->succ_next)
	free (e->data);

      if (v->data)
	{
	  gimple_set_uid (RDGV_STMT (v), -1);
	  (RDGV_DATAREFS (v)).release ();
	  free (v->data);
	}
    }


// Source: tree-loop-distribution.c
// Lines 769-787
