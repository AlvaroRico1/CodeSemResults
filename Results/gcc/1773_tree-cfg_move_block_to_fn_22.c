move_block_to_fn (struct function *dest_cfun, basic_block bb,
		  basic_block after, bool update_edge_count_p,
		  struct move_stmt_d *d)
{
  struct control_flow_graph *cfg;
  edge_iterator ei;
  edge e;
  gimple_stmt_iterator si;
  unsigned old_len, new_len;

  /* Remove BB from dominance structures.  */
  delete_from_dominance_info (CDI_DOMINATORS, bb);

  /* Move BB from its current loop to the copy in the new function.  */
  if (current_loops)
    {
      class loop *new_loop = (class loop *)bb->loop_father->aux;
      if (new_loop)
	bb->loop_father = new_loop;
    }


// Source: tree-cfg.c
// Lines 7103-7122
