bool Query_block::resolve_subquery(THD *thd) {
  DBUG_TRACE;

  bool choice_made = false;  // becomes true when subquery strategy is chosen
  bool deterministic = true;
  Query_block *const outer = outer_query_block();

  /*
    @todo for PS, make the whole block execute only on the first execution.
    resolve_subquery() is only invoked in the first execution for subqueries
    that are transformed to semijoin, but for other subqueries, this function
    is called for every execution. One solution is perhaps to define
    exec_method in class Item_subselect and exit immediately if unequal to
    SubqueryExecMethod::EXEC_UNSPECIFIED.
  */
  Item_subselect *subq_predicate = master_query_expression()->item;
  assert(subq_predicate != nullptr);
  /**
    @note
    In this case: IN (SELECT ... UNION SELECT ...), Query_block::prepare() is
    called for each of the two UNION members, and in those two calls,
    subq_predicate is the same, not sure this is desired (double work?).
  */

  // Predicate for possible semi-join candidates (IN and EXISTS)
  Item_exists_subselect *const predicate =
      subq_predicate->substype() == Item_subselect::EXISTS_SUBS ||
              subq_predicate->substype() == Item_subselect::IN_SUBS
          ? down_cast<Item_exists_subselect *>(subq_predicate)
          : nullptr;

  // Predicate for IN subquery predicate
  Item_in_subselect *const in_predicate =
      subq_predicate->substype() == Item_subselect::IN_SUBS
          ? down_cast<Item_in_subselect *>(subq_predicate)
          : nullptr;

  if (in_predicate != nullptr) {
    thd->lex->set_current_query_block(outer);
    char const *save_where = thd->where;
    thd->where = "IN/ALL/ANY subquery";
    Condition_context CCT(outer);

    bool result =
        !in_predicate->left_expr->fixed &&
        in_predicate->left_expr->fix_fields(thd, &in_predicate->left_expr);
    thd->lex->set_current_query_block(this);
    thd->where = save_where;
    if (result) return true;

    /*
      Check if the left and right expressions have the same # of
      columns, i.e. we don't have a case like
        (oe1, oe2) IN (SELECT ie1, ie2, ie3 ...)

      TODO why do we have this duplicated in IN->EXISTS transformers?
      psergey-todo: fix these: grep for duplicated_subselect_card_check
    */
    if (num_visible_fields() != in_predicate->left_expr->cols()) {
      my_error(ER_OPERAND_COLUMNS, MYF(0), in_predicate->left_expr->cols());
      return true;
    }
    if (in_predicate->left_expr->is_non_deterministic()) deterministic = false;
  }


// Source: sql_resolver.cc
// Lines 1326-1389
