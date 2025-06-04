compute_nearerout (struct edge_list *edge_list, sbitmap *farthest,
		   sbitmap *st_avloc, sbitmap *nearer, sbitmap *nearerout)
{
  int num_edges, i;
  edge e;
  basic_block *worklist, *tos, bb;
  edge_iterator ei;

  num_edges = NUM_EDGES (edge_list);

  /* Allocate a worklist array/queue.  Entries are only added to the
     list if they were not already on the list.  So the size is
     bounded by the number of basic blocks.  */
  tos = worklist = XNEWVEC (basic_block, n_basic_blocks_for_fn (cfun) + 1);

  /* Initialize NEARER for each edge and build a mapping from an edge to
     its index.  */
  for (i = 0; i < num_edges; i++)
    INDEX_EDGE (edge_list, i)->aux = (void *) (size_t) i;

  /* We want a maximal solution.  */
  bitmap_vector_ones (nearer, num_edges);

  /* Note that even though we want an optimistic setting of NEARER, we
     do not want to be overly optimistic.  Consider an incoming edge to
     the exit block.  That edge should always have a NEARER value the
     same as FARTHEST for that edge.  */
  FOR_EACH_EDGE (e, ei, EXIT_BLOCK_PTR_FOR_FN (cfun)->preds)
    bitmap_copy (nearer[(size_t)e->aux], farthest[(size_t)e->aux]);

  /* Add all the blocks to the worklist.  This prevents an early exit
     from the loop given our optimistic initialization of NEARER.  */
  FOR_EACH_BB_FN (bb, cfun)
    {
      *tos++ = bb;
      bb->aux = bb;
    }

  /* Iterate until the worklist is empty.  */
  while (tos != worklist)
    {
      /* Take the first entry off the worklist.  */
      bb = *--tos;
      bb->aux = NULL;

      /* Compute the intersection of NEARER for each outgoing edge from B.  */
      bitmap_ones (nearerout[bb->index]);
      FOR_EACH_EDGE (e, ei, bb->succs)
	bitmap_and (nearerout[bb->index], nearerout[bb->index],
			 nearer[(size_t) e->aux]);

      /* Calculate NEARER for all incoming edges.  */
      FOR_EACH_EDGE (e, ei, bb->preds)
	if (bitmap_ior_and_compl (nearer[(size_t) e->aux],
				      farthest[(size_t) e->aux],
				      nearerout[e->dest->index],
				      st_avloc[e->dest->index])
	    /* If NEARER for an incoming edge was changed, then we need
	       to add the source of the incoming edge to the worklist.  */
	    && e->src != ENTRY_BLOCK_PTR_FOR_FN (cfun) && e->src->aux == 0)
	  {
	    *tos++ = e->src;
	    e->src->aux = e;
	  }
    }

  /* Computation of insertion and deletion points requires computing NEAREROUT
     for the ENTRY block.  We allocated an extra entry in the NEAREROUT array
     for just this purpose.  */
  bitmap_ones (nearerout[last_basic_block_for_fn (cfun)]);
  FOR_EACH_EDGE (e, ei, ENTRY_BLOCK_PTR_FOR_FN (cfun)->succs)
    bitmap_and (nearerout[last_basic_block_for_fn (cfun)],
		     nearerout[last_basic_block_for_fn (cfun)],
		     nearer[(size_t) e->aux]);

  clear_aux_for_edges ();
  free (tos);
}


// Source: lcm.c
// Lines 623-700
