hsa_brig_emit_function (void)
{
  basic_block bb, prev_bb;
  hsa_insn_basic *insn;
  BrigDirectiveExecutable *ptr_to_fndir;

  brig_init ();

  brig_insn_count = 0;
  memset (&op_queue, 0, sizeof (op_queue));
  op_queue.projected_size = brig_operand.total_size;

  if (!function_offsets)
    function_offsets = new hash_map<tree, BrigCodeOffset32_t> ();

  if (!emitted_declarations)
    emitted_declarations = new hash_map <tree, BrigDirectiveExecutable *> ();

  for (unsigned i = 0; i < hsa_cfun->m_called_functions.length (); i++)
    {
      tree called = hsa_cfun->m_called_functions[i];

      /* If the function has no definition, emit a declaration.  */
      if (!emitted_declarations->get (called))
	{
	  BrigDirectiveExecutable *e = emit_function_declaration (called);
	  emitted_declarations->put (called, e);
	}
    }

  for (unsigned i = 0; i < hsa_cfun->m_called_internal_fns.length (); i++)
    {
      hsa_internal_fn *called = hsa_cfun->m_called_internal_fns[i];
      emit_internal_fn_decl (called);
    }

  ptr_to_fndir = emit_function_directives (hsa_cfun, false);
  for (insn = hsa_bb_for_bb (ENTRY_BLOCK_PTR_FOR_FN (cfun))->m_first_insn;
       insn;
       insn = insn->m_next)
    emit_insn (insn);
  prev_bb = ENTRY_BLOCK_PTR_FOR_FN (cfun);
  FOR_EACH_BB_FN (bb, cfun)
    {
      perhaps_emit_branch (prev_bb, bb);
      emit_bb_label_directive (hsa_bb_for_bb (bb));
      for (insn = hsa_bb_for_bb (bb)->m_first_insn; insn; insn = insn->m_next)
	emit_insn (insn);
      prev_bb = bb;
    }
  perhaps_emit_branch (prev_bb, NULL);
  ptr_to_fndir->nextModuleEntry = lendian32 (brig_code.total_size);

  /* Fill up label references for all sbr instructions.  */
  if (switch_instructions)
    {
      for (unsigned i = 0; i < switch_instructions->length (); i++)
	{
	  hsa_insn_sbr *sbr = (*switch_instructions)[i];
	  for (unsigned j = 0; j < sbr->m_jump_table.length (); j++)
	    {
	      hsa_bb *hbb = hsa_bb_for_bb (sbr->m_jump_table[j]);
	      sbr->m_label_code_list->m_offsets[j]
		= hbb->m_label_ref.m_directive_offset;
	    }
	}

      switch_instructions->release ();
      delete switch_instructions;
      switch_instructions = NULL;
    }

  if (dump_file)
    {
      fprintf (dump_file, "------- After BRIG emission: -------\n");
      dump_hsa_cfun (dump_file);
    }

  emit_queued_operands ();
}


// Source: hsa-brig.c
// Lines 1977-2056
