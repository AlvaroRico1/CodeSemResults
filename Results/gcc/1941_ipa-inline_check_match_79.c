#define check_match(flag) \
      (opts_for_fn (caller->decl)->x_##flag		\
       != opts_for_fn (callee->decl)->x_##flag)

/* Decide if we can inline the edge and possibly update
   inline_failed reason.  
   We check whether inlining is possible at all and whether
   caller growth limits allow doing so.  

   if REPORT is true, output reason to the dump file. */

static bool
can_inline_edge_p (struct cgraph_edge *e, bool report,
		   bool early = false)
{
  gcc_checking_assert (e->inline_failed);

  if (cgraph_inline_failed_type (e->inline_failed) == CIF_FINAL_ERROR)
    {
      if (report)
        report_inline_failed_reason (e);
      return false;
    }

  bool inlinable = true;
  enum availability avail;
  cgraph_node *caller = (e->caller->inlined_to
			 ? e->caller->inlined_to : e->caller);
  cgraph_node *callee = e->callee->ultimate_alias_target (&avail, caller);

  if (!callee->definition)
    {
      e->inline_failed = CIF_BODY_NOT_AVAILABLE;
      inlinable = false;
    }
  if (!early && (!opt_for_fn (callee->decl, optimize)
		 || !opt_for_fn (caller->decl, optimize)))
    {
      e->inline_failed = CIF_FUNCTION_NOT_OPTIMIZED;
      inlinable = false;
    }
  else if (callee->calls_comdat_local)
    {
      e->inline_failed = CIF_USES_COMDAT_LOCAL;
      inlinable = false;
    }
  else if (avail <= AVAIL_INTERPOSABLE)
    {
      e->inline_failed = CIF_OVERWRITABLE;
      inlinable = false;
    }
  /* All edges with call_stmt_cannot_inline_p should have inline_failed
     initialized to one of FINAL_ERROR reasons.  */
  else if (e->call_stmt_cannot_inline_p)
    gcc_unreachable ();
  /* Don't inline if the functions have different EH personalities.  */
  else if (DECL_FUNCTION_PERSONALITY (caller->decl)
	   && DECL_FUNCTION_PERSONALITY (callee->decl)
	   && (DECL_FUNCTION_PERSONALITY (caller->decl)
	       != DECL_FUNCTION_PERSONALITY (callee->decl)))
    {
      e->inline_failed = CIF_EH_PERSONALITY;
      inlinable = false;
    }
  /* TM pure functions should not be inlined into non-TM_pure
     functions.  */
  else if (is_tm_pure (callee->decl) && !is_tm_pure (caller->decl))
    {
      e->inline_failed = CIF_UNSPECIFIED;
      inlinable = false;
    }
  /* Check compatibility of target optimization options.  */
  else if (!targetm.target_option.can_inline_p (caller->decl,
						callee->decl))
    {
      e->inline_failed = CIF_TARGET_OPTION_MISMATCH;
      inlinable = false;
    }
  else if (ipa_fn_summaries->get (callee) == NULL
	   || !ipa_fn_summaries->get (callee)->inlinable)
    {
      e->inline_failed = CIF_FUNCTION_NOT_INLINABLE;
      inlinable = false;
    }
  /* Don't inline a function with mismatched sanitization attributes. */
  else if (!sanitize_attrs_match_for_inline_p (caller->decl, callee->decl))
    {
      e->inline_failed = CIF_ATTRIBUTE_MISMATCH;
      inlinable = false;
    }
  if (!inlinable && report)
    report_inline_failed_reason (e);
  return inlinable;
}


// Source: ipa-inline.c
// Lines 298-391
