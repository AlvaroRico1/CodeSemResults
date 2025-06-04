single_use_in_loop (tree var, class loop *loop)
{
  gimple *stmt, *res = NULL;
  use_operand_p use_p;
  imm_use_iterator iterator;

  FOR_EACH_IMM_USE_FAST (use_p, iterator, var)
    {
      stmt = USE_STMT (use_p);
      if (is_gimple_debug (stmt))
	continue;

      if (!flow_bb_inside_loop_p (loop, gimple_bb (stmt)))
	continue;

      if (res)
	return NULL;

      res = stmt;
    }
  return res;
}


// Source: gimple-loop-interchange.cc
// Lines 243-264
