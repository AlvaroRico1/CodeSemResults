update_edge_key (edge_heap_t *heap, struct cgraph_edge *edge)
{
  sreal badness = edge_badness (edge, false);
  if (edge->aux)
    {
      edge_heap_node_t *n = (edge_heap_node_t *) edge->aux;
      gcc_checking_assert (n->get_data () == edge);

      /* fibonacci_heap::replace_key does busy updating of the
	 heap that is unnecessarily expensive.
	 We do lazy increases: after extracting minimum if the key
	 turns out to be out of date, it is re-inserted into heap
	 with correct value.  */
      if (badness < n->get_key ())
	{
	  if (dump_file && (dump_flags & TDF_DETAILS))
	    {
	      fprintf (dump_file,
		       "  decreasing badness %s -> %s, %f to %f\n",
		       edge->caller->dump_name (),
		       edge->callee->dump_name (),
		       n->get_key ().to_double (),
		       badness.to_double ());
	    }
	  heap->decrease_key (n, badness);
	}
    }


// Source: ipa-inline.c
// Lines 1329-1355
