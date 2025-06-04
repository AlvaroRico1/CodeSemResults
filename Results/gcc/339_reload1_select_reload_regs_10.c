select_reload_regs (void)
{
  class insn_chain *chain;

  /* Try to satisfy the needs for each insn.  */
  for (chain = insns_need_reload; chain != 0;
       chain = chain->next_need_reload)
    find_reload_regs (chain);
}


// Source: reload1.c
// Lines 2013-2021
