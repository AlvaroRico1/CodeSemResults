redirect_immediate_dominators (enum cdi_direction dir, basic_block bb,
			       basic_block to)
{
  unsigned int dir_index = dom_convert_dir_to_idx (dir);
  struct et_node *bb_node, *to_node, *son;

  bb_node = bb->dom[dir_index];
  to_node = to->dom[dir_index];

  gcc_checking_assert (dom_computed[dir_index]);

  if (!bb_node->son)
    return;

  while (bb_node->son)
    {
      son = bb_node->son;

      et_split (son);
      et_set_father (son, to_node);
    }

  if (dom_computed[dir_index] == DOM_OK)
    dom_computed[dir_index] = DOM_NO_FAST_QUERY;
}


// Source: dominance.c
// Lines 976-1000
