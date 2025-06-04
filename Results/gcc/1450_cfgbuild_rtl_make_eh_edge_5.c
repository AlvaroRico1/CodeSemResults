rtl_make_eh_edge (sbitmap edge_cache, basic_block src, rtx insn)
{
  eh_landing_pad lp = get_eh_landing_pad_from_rtx (insn);

  if (lp)
    {
      rtx_insn *label = lp->landing_pad;

      /* During initial rtl generation, use the post_landing_pad.  */
      if (label == NULL)
	{
	  gcc_assert (lp->post_landing_pad);
	  label = label_rtx (lp->post_landing_pad);
	}

      make_label_edge (edge_cache, src, label,
		       EDGE_ABNORMAL | EDGE_EH
		       | (CALL_P (insn) ? EDGE_ABNORMAL_CALL : 0));
    }


// Source: cfgbuild.c
// Lines 146-164
