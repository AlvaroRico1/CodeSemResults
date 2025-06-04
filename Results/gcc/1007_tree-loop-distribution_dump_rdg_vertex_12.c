dump_rdg_vertex (FILE *file, struct graph *rdg, int i)
{
  struct vertex *v = &(rdg->vertices[i]);
  struct graph_edge *e;

  fprintf (file, "(vertex %d: (%s%s) (in:", i,
	   RDG_MEM_WRITE_STMT (rdg, i) ? "w" : "",
	   RDG_MEM_READS_STMT (rdg, i) ? "r" : "");

  if (v->pred)
    for (e = v->pred; e; e = e->pred_next)
      fprintf (file, " %d", e->src);

  fprintf (file, ") (out:");

  if (v->succ)
    for (e = v->succ; e; e = e->succ_next)
      fprintf (file, " %d", e->dest);

  fprintf (file, ")\n");
  print_gimple_stmt (file, RDGV_STMT (v), 0, TDF_VOPS|TDF_MEMSYMS);
  fprintf (file, ")\n");
}


// Source: tree-loop-distribution.c
// Lines 287-309
