static void end_lex_with_single_table(THD *thd, TABLE *table, LEX *old_lex) {
  LEX *lex = thd->lex;
  table->get_fields_in_item_tree = false;
  lex_end(lex);
  thd->lex = old_lex;
}


// Source: sql_partition.cc
// Lines 875-880
