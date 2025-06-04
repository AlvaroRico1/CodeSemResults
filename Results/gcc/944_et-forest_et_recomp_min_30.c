et_recomp_min (struct et_occ *occ)
{
  struct et_occ *mson = occ->prev;

  if (!mson
      || (occ->next
	  && mson->min > occ->next->min))
      mson = occ->next;

  if (mson && mson->min < 0)
    {
      occ->min = mson->min + occ->depth;
      occ->min_occ = mson->min_occ;
    }
  else
    {
      occ->min = occ->depth;
      occ->min_occ = occ;
    }
}


// Source: et-forest.c
// Lines 116-135
