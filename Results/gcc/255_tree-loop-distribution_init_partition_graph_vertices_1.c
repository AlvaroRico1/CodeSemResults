init_partition_graph_vertices (struct graph *pg,
			       vec<struct partition *> *partitions)
{
  int i;
  partition *partition;
  struct pg_vdata *data;

  for (i = 0; partitions->iterate (i, &partition); ++i)
    {
      data = new pg_vdata;
      pg->vertices[i].data = data;
      data->id = i;
      data->partition = partition;
    }
}


// Source: tree-loop-distribution.c
// Lines 2159-2173
