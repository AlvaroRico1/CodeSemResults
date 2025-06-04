dump_inline_stats (void)
{
  int64_t inlined_cnt = 0, inlined_indir_cnt = 0;
  int64_t inlined_virt_cnt = 0, inlined_virt_indir_cnt = 0;
  int64_t noninlined_cnt = 0, noninlined_indir_cnt = 0;
  int64_t noninlined_virt_cnt = 0, noninlined_virt_indir_cnt = 0;
  int64_t  inlined_speculative = 0, inlined_speculative_ply = 0;
  int64_t indirect_poly_cnt = 0, indirect_cnt = 0;
  int64_t reason[CIF_N_REASONS][2];
  sreal reason_freq[CIF_N_REASONS];
  int i;
  struct cgraph_node *node;

  memset (reason, 0, sizeof (reason));
  for (i=0; i < CIF_N_REASONS; i++)
    reason_freq[i] = 0;
  FOR_EACH_DEFINED_FUNCTION (node)
  {
    struct cgraph_edge *e;
    for (e = node->callees; e; e = e->next_callee)
      {
	if (e->inline_failed)
	  {
	    if (e->count.ipa ().initialized_p ())
	      reason[(int) e->inline_failed][0] += e->count.ipa ().to_gcov_type ();
	    reason_freq[(int) e->inline_failed] += e->sreal_frequency ();
	    reason[(int) e->inline_failed][1] ++;
	    if (DECL_VIRTUAL_P (e->callee->decl)
		&& e->count.ipa ().initialized_p ())
	      {
		if (e->indirect_inlining_edge)
		  noninlined_virt_indir_cnt += e->count.ipa ().to_gcov_type ();
		else
		  noninlined_virt_cnt += e->count.ipa ().to_gcov_type ();
	      }
	    else if (e->count.ipa ().initialized_p ())
	      {
		if (e->indirect_inlining_edge)
		  noninlined_indir_cnt += e->count.ipa ().to_gcov_type ();
		else
		  noninlined_cnt += e->count.ipa ().to_gcov_type ();
	      }
	  }
	else if (e->count.ipa ().initialized_p ())
	  {
	    if (e->speculative)
	      {
		if (DECL_VIRTUAL_P (e->callee->decl))
		  inlined_speculative_ply += e->count.ipa ().to_gcov_type ();
		else
		  inlined_speculative += e->count.ipa ().to_gcov_type ();
	      }
	    else if (DECL_VIRTUAL_P (e->callee->decl))
	      {
		if (e->indirect_inlining_edge)
		  inlined_virt_indir_cnt += e->count.ipa ().to_gcov_type ();
		else
		  inlined_virt_cnt += e->count.ipa ().to_gcov_type ();
	      }
	    else
	      {
		if (e->indirect_inlining_edge)
		  inlined_indir_cnt += e->count.ipa ().to_gcov_type ();
		else
		  inlined_cnt += e->count.ipa ().to_gcov_type ();
	      }
	  }
      }
    for (e = node->indirect_calls; e; e = e->next_callee)
      if (e->indirect_info->polymorphic
	  & e->count.ipa ().initialized_p ())
	indirect_poly_cnt += e->count.ipa ().to_gcov_type ();
      else if (e->count.ipa ().initialized_p ())
	indirect_cnt += e->count.ipa ().to_gcov_type ();
  }


// Source: ipa-inline.c
// Lines 2470-2544
