set_comdat_group (symtab_node *symbol,
	          void *head_p)
{
  symtab_node *head = (symtab_node *)head_p;

  gcc_assert (!symbol->get_comdat_group ());
  if (symbol->real_symbol_p ())
    {
      symbol->set_comdat_group (head->get_comdat_group ());
      symbol->add_to_same_comdat_group (head);
    }
  return false;
}


// Source: ipa-comdats.c
// Lines 208-220
