bb_top_order_cmp_r (const void *x, const void *y, void *loop)
{
  loop_distribution *_loop =
    (loop_distribution *) loop;

  basic_block bb1 = *(const basic_block *) x;
  basic_block bb2 = *(const basic_block *) y;

  int bb_top_order_index_size = _loop->get_bb_top_order_index_size ();

  gcc_assert (bb1->index < bb_top_order_index_size
	      && bb2->index < bb_top_order_index_size);
  gcc_assert (bb1 == bb2
	      || _loop->get_bb_top_order_index(bb1->index)
		 != _loop->get_bb_top_order_index(bb2->index));

  return (_loop->get_bb_top_order_index(bb1->index) - 
	  _loop->get_bb_top_order_index(bb2->index));
}


// Source: tree-loop-distribution.c
// Lines 680-698
