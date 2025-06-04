print_insn_chains (FILE *file)
{
  class insn_chain *c;
  for (c = reload_insn_chain; c ; c = c->next)
    print_insn_chain (file, c);
}


// Source: ira.c
// Lines 4081-4086
