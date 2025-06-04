cleanup_dead_labels_eh (label_record *label_for_bb)
{
  eh_landing_pad lp;
  eh_region r;
  tree lab;
  int i;

  if (cfun->eh == NULL)
    return;

  for (i = 1; vec_safe_iterate (cfun->eh->lp_array, i, &lp); ++i)
    if (lp && lp->post_landing_pad)
      {
	lab = main_block_label (lp->post_landing_pad, label_for_bb);
	if (lab != lp->post_landing_pad)
	  {
	    EH_LANDING_PAD_NR (lp->post_landing_pad) = 0;
	    EH_LANDING_PAD_NR (lab) = lp->index;
	  }
      }

  FOR_ALL_EH_REGION (r)
    switch (r->type)
      {
      case ERT_CLEANUP:
      case ERT_MUST_NOT_THROW:
	break;

      case ERT_TRY:
	{
	  eh_catch c;
	  for (c = r->u.eh_try.first_catch; c ; c = c->next_catch)
	    {
	      lab = c->label;
	      if (lab)
		c->label = main_block_label (lab, label_for_bb);
	    }
	}


// Source: tree-cfg.c
// Lines 1474-1511
