remove_bb (basic_block bb)
{
  gimple_stmt_iterator i;

  if (dump_file)
    {
      fprintf (dump_file, "Removing basic block %d\n", bb->index);
      if (dump_flags & TDF_DETAILS)
	{
	  dump_bb (dump_file, bb, 0, TDF_BLOCKS);
	  fprintf (dump_file, "\n");
	}
    }

  if (current_loops)
    {
      class loop *loop = bb->loop_father;

      /* If a loop gets removed, clean up the information associated
	 with it.  */
      if (loop->latch == bb
	  || loop->header == bb)
	free_numbers_of_iterations_estimates (loop);
    }


// Source: tree-cfg.c
// Lines 2254-2277
