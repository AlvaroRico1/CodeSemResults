              [&](Item **expr_p) mutable -> bool {
                subqueries.m_location =
                    Item::Collect_scalar_subquery_info::L_JOIN_COND;
                if (*expr_p != nullptr &&
                    replace_subquery_in_expr(thd, &subquery, tl, expr_p))
                  return true; /* purecov: inspected */
                return false;
              },
              &subqueries))
        return true; /* purecov: inspected */
    }

    size_t old_size;
    do {
      old_size = fields.size();
      for (Item *&select_expr : fields) {
        Item *prev_value = select_expr;
        if (replace_subquery_in_expr(thd, &subquery, tl, &select_expr))
          return true;
        if (select_expr != prev_value) {
          for (size_t i = 0; i < fields.size(); i++) {
            if (base_ref_items[i] == prev_value)
              base_ref_items[i] = select_expr;
          }
        }
        if (fields.size() != old_size) {
          // The (implicit) iterator over fields has been invalidated,
          // probably due to a call to split_sum_func(), so we cannot
          // iterate any further. The simplest fix is just restarting
          // the loop, as it is idempotent.
          break;
        }
      }
    } while (old_size != fields.size());

    // Replace in HAVING clause?
    if (subquery.m_location & (Item::Collect_scalar_subquery_info::L_HAVING)) {
      if (*having_expr_p != nullptr &&
          replace_subquery_in_expr(thd, &subquery, tl, having_expr_p))
        return true; /* purecov: inspected */
    }

    // A subquery in the SELECT list can be present in the GROUP BY clause
    // so we potentially need to replace there too.
    for (ORDER *ord = group_list.first; ord != nullptr; ord = ord->next) {
      if (replace_subquery_in_expr(thd, &subquery, tl, ord->item)) return true;
    }

    OPT_TRACE_TRANSFORM(
        &thd->opt_trace, trace_wrapper, trace_object,
        tl->derived_query_expression()->first_query_block()->select_number,
        "scalar subquery", "derived table");
    opt_trace_print_expanded_query(thd, this, &trace_object);
  }


// Source: sql_resolver.cc
// Lines 7436-7489
