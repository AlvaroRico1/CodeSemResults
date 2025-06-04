delete_caller_save_insns (void)
{
  class insn_chain *c = reload_insn_chain;

  while (c != 0)
    {
      while (c != 0 && c->is_caller_save_insn)
	{
	  class insn_chain *next = c->next;
	  rtx_insn *insn = c->insn;

	  if (c == reload_insn_chain)
	    reload_insn_chain = next;
	  delete_insn (insn);

	  if (next)
	    next->prev = c->prev;
	  if (c->prev)
	    c->prev->next = next;
	  c->next = unused_insn_chains;
	  unused_insn_chains = c;
	  c = next;
	}
      if (c != 0)
	c = c->next;
    }
}


// Source: reload1.c
// Lines 2026-2052
