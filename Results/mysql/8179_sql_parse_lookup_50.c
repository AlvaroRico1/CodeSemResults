bool PT_with_clause::lookup(TABLE_LIST *tl, PT_common_table_expr **found) {
  *found = nullptr;
  assert(tl->query_block != nullptr);
  /*
    If right_bound!=NULL, it means we are currently parsing the
    definition of CTE 'right_bound' and this definition contains
    'tl'.
  */
  const Common_table_expr *right_bound =
      m_most_inner_in_parsing ? m_most_inner_in_parsing->common_table_expr()
                              : nullptr;
  bool in_self = false;
  for (auto el : m_list->elements()) {
    // Search for a CTE named like 'tl', in this list, from left to right.
    if (el->is(right_bound)) {
      /*
        We meet right_bound.
        If not RECURSIVE:
        we must stop the search in this WITH clause;
        indeed right_bound must not reference itself or any CTE defined after it
        in the WITH list (forward references are forbidden, preventing any
        cycle).
        If RECURSIVE:
        If right_bound matches 'tl', it is a recursive reference.
      */
      if (!m_recursive) break;  // Prevent forward reference.
      in_self = true;           // Accept a recursive reference.
    }
    bool match;
    if (el->match_table_ref(tl, in_self, &match)) return true;
    if (!match) {
      if (in_self) break;  // Prevent forward reference.
      continue;
    }
    if (in_self && tl->query_block->outer_query_block() !=
                       m_most_inner_in_parsing->query_block) {
      /*
        SQL2011 says a recursive CTE cannot contain a subquery
        referencing the CTE, except if this subquery is a derived table
        like:
        WITH RECURSIVE qn AS (non-rec-SELECT UNION ALL
        SELECT * FROM (SELECT * FROM qn) AS dt)
        However, we don't allow this, as:
        - it simplifies detection and substitution correct recursive
        references (they're all on "level 0" of the UNION)
        - it's not limiting the user so much (in most cases, he can just
        merge his DT up manually, as the DT cannot contain aggregation).
        - Oracle bans it:
        with qn (a) as (
        select 123 from dual
        union all
        select 1+a from (select * from qn) where a<130) select * from qn
        ORA-32042: recursive WITH clause must reference itself directly in one
        of the UNION ALL branches.

        The above if() works because, when we parse such example query, we
        first resolve the 'qn' reference in the top query, making it a derived
        table:

        select * from (
           select 123 from dual
           union all
           select 1+a from (select * from qn) where a<130) qn(a);
                                                           ^most_inner_in_parsing
        Then we contextualize that derived table (containing the union);
        when we contextualize the recursive query block of the union, the
        inner 'qn' is recognized as a recursive reference, and its
        query_block->outer_query_block() is _not_ the query_block of
        most_inner_in_parsing, which indicates that the inner 'qn' is placed
        too deep.
      */
      my_error(ER_CTE_RECURSIVE_REQUIRES_SINGLE_REFERENCE, MYF(0),
               el->name().str);
      return true;
    }
    *found = el;
    break;
  }
  return false;
}


// Source: sql_parse.cc
// Lines 5438-5517
