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


// Source: hsa-gen.c
// Lines 848-894
