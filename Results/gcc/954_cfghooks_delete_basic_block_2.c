delete_basic_block (basic_block bb)
{
  if (!cfg_hooks->delete_basic_block)
    internal_error ("%s does not support delete_basic_block", cfg_hooks->name);

  cfg_hooks->delete_basic_block (bb);

  if (current_loops != NULL)
    {
      class loop *loop = bb->loop_father;

      /* If we remove the header or the latch of a loop, mark the loop for
	 removal.  */
      if (loop->latch == bb
	  || loop->header == bb)
	mark_loop_for_removal (loop);

      remove_bb_from_loops (bb);
    }


// Source: cfghooks.c
// Lines 598-616
