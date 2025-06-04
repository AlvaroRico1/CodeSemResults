static bool init_param_array(Prepared_statement *stmt) {
  LEX *lex = stmt->lex;
  if ((stmt->param_count = lex->param_list.elements)) {
    if (stmt->param_count > (uint)UINT_MAX16) {
      /* Error code to be defined in 5.0 */
      my_error(ER_PS_MANY_PARAM, MYF(0));
      return true;
    }

    Item_param **to;
    List_iterator<Item_param> param_iterator(lex->param_list);
    /* Use thd->mem_root as it points at statement mem_root */
    stmt->param_array =
        stmt->thd->mem_root->ArrayAlloc<Item_param *>(stmt->param_count);
    if (stmt->param_array == nullptr) return true;
    for (to = stmt->param_array; to < stmt->param_array + stmt->param_count;
         ++to) {
      *to = param_iterator++;
    }
  }
  return false;
}


// Source: sql_prepare.cc
// Lines 1499-1520
