inline_read_section (struct lto_file_decl_data *file_data, const char *data,
		     size_t len)
{
  const struct lto_function_header *header =
    (const struct lto_function_header *) data;
  const int cfg_offset = sizeof (struct lto_function_header);
  const int main_offset = cfg_offset + header->cfg_size;
  const int string_offset = main_offset + header->main_size;
  class data_in *data_in;
  unsigned int i, count2, j;
  unsigned int f_count;

  lto_input_block ib ((const char *) data + main_offset, header->main_size,
		      file_data->mode_table);

  data_in =
    lto_data_in_create (file_data, (const char *) data + string_offset,
			header->string_size, vNULL);
  f_count = streamer_read_uhwi (&ib);
  for (i = 0; i < f_count; i++)
    {
      unsigned int index;
      struct cgraph_node *node;
      class ipa_fn_summary *info;
      class ipa_node_params *params_summary;
      class ipa_size_summary *size_info;
      lto_symtab_encoder_t encoder;
      struct bitpack_d bp;
      struct cgraph_edge *e;
      predicate p;

      index = streamer_read_uhwi (&ib);
      encoder = file_data->symtab_node_encoder;
      node = dyn_cast<cgraph_node *> (lto_symtab_encoder_deref (encoder,
								index));
      info = node->prevailing_p () ? ipa_fn_summaries->get_create (node) : NULL;
      params_summary = node->prevailing_p () ? IPA_NODE_REF (node) : NULL;
      size_info = node->prevailing_p ()
		  ? ipa_size_summaries->get_create (node) : NULL;

      int stack_size = streamer_read_uhwi (&ib);
      int size = streamer_read_uhwi (&ib);
      sreal time = sreal::stream_in (&ib);

      if (info)
	{
	  info->estimated_stack_size
	    = size_info->estimated_self_stack_size = stack_size;
	  size_info->size = size_info->self_size = size;
	  info->time = time;
	}

      bp = streamer_read_bitpack (&ib);
      if (info)
	{
          info->inlinable = bp_unpack_value (&bp, 1);
          info->fp_expressions = bp_unpack_value (&bp, 1);
	}
      else
	{
          bp_unpack_value (&bp, 1);
          bp_unpack_value (&bp, 1);
	}

      count2 = streamer_read_uhwi (&ib);
      gcc_assert (!info || !info->conds);
      if (info)
        vec_safe_reserve_exact (info->conds, count2);
      for (j = 0; j < count2; j++)
	{
	  struct condition c;
	  unsigned int k, count3;
	  c.operand_num = streamer_read_uhwi (&ib);
	  c.code = (enum tree_code) streamer_read_uhwi (&ib);
	  c.type = stream_read_tree (&ib, data_in);
	  c.val = stream_read_tree (&ib, data_in);
	  bp = streamer_read_bitpack (&ib);
	  c.agg_contents = bp_unpack_value (&bp, 1);
	  c.by_ref = bp_unpack_value (&bp, 1);
	  if (c.agg_contents)
	    c.offset = streamer_read_uhwi (&ib);
	  count3 = streamer_read_uhwi (&ib);
	  c.param_ops = NULL;
	  if (info)
	    vec_safe_reserve_exact (c.param_ops, count3);
	  if (params_summary)
	    ipa_set_param_used_by_ipa_predicates
		    (params_summary, c.operand_num, true);
	  for (k = 0; k < count3; k++)
	    {
	      struct expr_eval_op op;
	      enum gimple_rhs_class rhs_class;
	      op.code = (enum tree_code) streamer_read_uhwi (&ib);
	      op.type = stream_read_tree (&ib, data_in);
	      switch (rhs_class = get_gimple_rhs_class (op.code))
		{
		case GIMPLE_UNARY_RHS:
		  op.index = 0;
		  op.val[0] = NULL_TREE;
		  op.val[1] = NULL_TREE;
		  break;

		case GIMPLE_BINARY_RHS:
		case GIMPLE_TERNARY_RHS:
		  bp = streamer_read_bitpack (&ib);
		  op.index = bp_unpack_value (&bp, 2);
		  op.val[0] = stream_read_tree (&ib, data_in);
		  if (rhs_class == GIMPLE_BINARY_RHS)
		    op.val[1] = NULL_TREE;
		  else
		    op.val[1] = stream_read_tree (&ib, data_in);
		  break;

		default:
		  fatal_error (UNKNOWN_LOCATION,
			       "invalid fnsummary in LTO stream");
		}
	      if (info)
	        c.param_ops->quick_push (op);
	    }
	  if (info)
	    info->conds->quick_push (c);
	}
      count2 = streamer_read_uhwi (&ib);
      gcc_assert (!info || !info->size_time_table);
      if (info && count2)
        vec_safe_reserve_exact (info->size_time_table, count2);
      for (j = 0; j < count2; j++)
	{
	  class size_time_entry e;

	  e.size = streamer_read_uhwi (&ib);
	  e.time = sreal::stream_in (&ib);
	  e.exec_predicate.stream_in (&ib);
	  e.nonconst_predicate.stream_in (&ib);

	  if (info)
	    info->size_time_table->quick_push (e);
	}

      p.stream_in (&ib);
      if (info)
        set_hint_predicate (&info->loop_iterations, p);
      p.stream_in (&ib);
      if (info)
        set_hint_predicate (&info->loop_stride, p);
      for (e = node->callees; e; e = e->next_callee)
	read_ipa_call_summary (&ib, e, info != NULL);
      for (e = node->indirect_calls; e; e = e->next_callee)
	read_ipa_call_summary (&ib, e, info != NULL);
    }


// Source: ipa-fnsummary.c
// Lines 4187-4337
