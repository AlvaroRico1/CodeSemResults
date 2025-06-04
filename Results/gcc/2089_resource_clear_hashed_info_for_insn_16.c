clear_hashed_info_for_insn (rtx_insn *insn)
{
  struct target_info *tinfo;

  if (target_hash_table != NULL)
    {
      for (tinfo = target_hash_table[INSN_UID (insn) % TARGET_HASH_PRIME];
	   tinfo; tinfo = tinfo->next)
	if (tinfo->uid == INSN_UID (insn))
	  break;

      if (tinfo)
	tinfo->block = -1;
    }
}


// Source: resource.c
// Lines 1270-1284
