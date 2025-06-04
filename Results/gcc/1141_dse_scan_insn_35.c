scan_insn (bb_info_t bb_info, rtx_insn *insn, int max_active_local_stores)
{
  rtx body;
  insn_info_type *insn_info = insn_info_type_pool.allocate ();
  int mems_found = 0;
  memset (insn_info, 0, sizeof (struct insn_info_type));

  if (dump_file && (dump_flags & TDF_DETAILS))
    fprintf (dump_file, "\n**scanning insn=%d\n",
	     INSN_UID (insn));

  insn_info->prev_insn = bb_info->last_insn;
  insn_info->insn = insn;
  bb_info->last_insn = insn_info;

  if (DEBUG_INSN_P (insn))
    {
      insn_info->cannot_delete = true;
      return;
    }

  /* Look at all of the uses in the insn.  */
  note_uses (&PATTERN (insn), check_mem_read_use, bb_info);

  if (CALL_P (insn))
    {
      bool const_call;
      rtx call, sym;
      tree memset_call = NULL_TREE;

      insn_info->cannot_delete = true;

      /* Const functions cannot do anything bad i.e. read memory,
	 however, they can read their parameters which may have
	 been pushed onto the stack.
	 memset and bzero don't read memory either.  */
      const_call = RTL_CONST_CALL_P (insn);
      if (!const_call
	  && (call = get_call_rtx_from (insn))
	  && (sym = XEXP (XEXP (call, 0), 0))
	  && GET_CODE (sym) == SYMBOL_REF
	  && SYMBOL_REF_DECL (sym)
	  && TREE_CODE (SYMBOL_REF_DECL (sym)) == FUNCTION_DECL
	  && fndecl_built_in_p (SYMBOL_REF_DECL (sym), BUILT_IN_MEMSET))
	memset_call = SYMBOL_REF_DECL (sym);

      if (const_call || memset_call)
	{
	  insn_info_t i_ptr = active_local_stores;
	  insn_info_t last = NULL;

	  if (dump_file && (dump_flags & TDF_DETAILS))
	    fprintf (dump_file, "%s call %d\n",
		     const_call ? "const" : "memset", INSN_UID (insn));

	  /* See the head comment of the frame_read field.  */
	  if (reload_completed
	      /* Tail calls are storing their arguments using
		 arg pointer.  If it is a frame pointer on the target,
		 even before reload we need to kill frame pointer based
		 stores.  */
	      || (SIBLING_CALL_P (insn)
		  && HARD_FRAME_POINTER_IS_ARG_POINTER))
	    insn_info->frame_read = true;

	  /* Loop over the active stores and remove those which are
	     killed by the const function call.  */
	  while (i_ptr)
	    {
	      bool remove_store = false;

	      /* The stack pointer based stores are always killed.  */
	      if (i_ptr->stack_pointer_based)
	        remove_store = true;

	      /* If the frame is read, the frame related stores are killed.  */
	      else if (insn_info->frame_read)
		{
		  store_info *store_info = i_ptr->store_rec;

		  /* Skip the clobbers.  */
		  while (!store_info->is_set)
		    store_info = store_info->next;

		  if (store_info->group_id >= 0
		      && rtx_group_vec[store_info->group_id]->frame_related)
		    remove_store = true;
		}


// Source: dse.c
// Lines 2456-2543
