get_symbol_for_decl (tree decl)
{
  hsa_symbol **slot;
  hsa_symbol dummy (BRIG_TYPE_NONE, BRIG_SEGMENT_NONE, BRIG_LINKAGE_NONE);

  gcc_assert (TREE_CODE (decl) == PARM_DECL
	      || TREE_CODE (decl) == RESULT_DECL
	      || TREE_CODE (decl) == VAR_DECL
	      || TREE_CODE (decl) == CONST_DECL);

  dummy.m_decl = decl;

  bool is_in_global_vars = ((TREE_CODE (decl) == VAR_DECL)
			    && !decl_function_context (decl));

  if (is_in_global_vars)
    slot = hsa_global_variable_symbols->find_slot (&dummy, INSERT);
  else
    slot = hsa_cfun->m_local_symbols->find_slot (&dummy, INSERT);

  gcc_checking_assert (slot);
  if (*slot)
    {
      hsa_symbol *sym = (*slot);

      /* If the symbol is problematic, mark current function also as
	 problematic.  */
      if (sym->m_seen_error)
	hsa_fail_cfun ();

      /* PR hsa/70234: If a global variable was marked to be emitted,
	 but HSAIL generation of a function using the variable fails,
	 we should retry to emit the variable in context of a different
	 function.

	 Iterate elements whether a symbol is already in m_global_symbols
	 of not.  */
        if (is_in_global_vars && !sym->m_emitted_to_brig)
	  {
	    for (unsigned i = 0; i < hsa_cfun->m_global_symbols.length (); i++)
	      if (hsa_cfun->m_global_symbols[i] == sym)
		return *slot;
	    hsa_cfun->m_global_symbols.safe_push (sym);
	  }

      return *slot;
    }
  else
    {
      hsa_symbol *sym;
      /* PARM_DECLs and RESULT_DECL should be already in m_local_symbols.  */
      gcc_assert (TREE_CODE (decl) == VAR_DECL
		  || TREE_CODE (decl) == CONST_DECL);
      BrigAlignment8_t align = hsa_object_alignment (decl);

      if (is_in_global_vars)
	{
	  gcc_checking_assert (TREE_CODE (decl) != CONST_DECL);
	  sym = new hsa_symbol (BRIG_TYPE_NONE, BRIG_SEGMENT_GLOBAL,
				BRIG_LINKAGE_PROGRAM, true,
				BRIG_ALLOCATION_PROGRAM, align);
	  hsa_cfun->m_global_symbols.safe_push (sym);
	  sym->fillup_for_decl (decl);
	  if (sym->m_align > align)
	    {
	      sym->m_seen_error = true;
	      HSA_SORRY_ATV (EXPR_LOCATION (decl),
			     "HSA specification requires that %E is at least "
			     "naturally aligned", decl);
	    }
	}
      else
	{
	  /* As generation of efficient memory copy instructions relies
	     on alignment greater or equal to 8 bytes,
	     we need to increase alignment of all aggregate types.. */
	  if (AGGREGATE_TYPE_P (TREE_TYPE (decl)))
	    align = MAX ((BrigAlignment8_t) BRIG_ALIGNMENT_8, align);

	  BrigAllocation allocation = BRIG_ALLOCATION_AUTOMATIC;
	  BrigSegment8_t segment;
	  if (TREE_CODE (decl) == CONST_DECL)
	    {
	      segment = BRIG_SEGMENT_READONLY;
	      allocation = BRIG_ALLOCATION_AGENT;
	    }
	  else if (lookup_attribute ("hsa_group_segment",
				     DECL_ATTRIBUTES (decl)))
	    segment = BRIG_SEGMENT_GROUP;
	  else if (TREE_STATIC (decl))
	    {
	      segment = BRIG_SEGMENT_GLOBAL;
	      allocation = BRIG_ALLOCATION_PROGRAM;
	    }
	  else if (lookup_attribute ("hsa_global_segment",
				     DECL_ATTRIBUTES (decl)))
	    segment = BRIG_SEGMENT_GLOBAL;
	  else
	    segment = BRIG_SEGMENT_PRIVATE;

	  sym = new hsa_symbol (BRIG_TYPE_NONE, segment, BRIG_LINKAGE_FUNCTION,
				false, allocation, align);
	  sym->fillup_for_decl (decl);
	  hsa_cfun->m_private_variables.safe_push (sym);
	}

      sym->m_name = hsa_get_declaration_name (decl);
      *slot = sym;
      return sym;
    }


// Source: hsa-gen.c
// Lines 848-957
