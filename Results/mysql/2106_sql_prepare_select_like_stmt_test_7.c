static bool select_like_stmt_test(THD *thd, Query_result *result,
                                  ulonglong added_options) {
  DBUG_TRACE;
  LEX *const lex = thd->lex;

  lex->query_block->context.resolve_in_select_list = true;

  if (lex->unit->prepare(thd, result, nullptr, added_options, 0)) {
    return true;
  }
  lex->save_cmd_properties(thd);

  return false;
}


// Source: sql_prepare.cc
// Lines 1100-1113
