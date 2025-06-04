static bool sj_table_is_included(JOIN *join, JOIN_TAB *join_tab) {
  if (join_tab->emb_sj_nest) return false;

  /* Check if this table is functionally dependent on the tables that
     are within the same outer join nest
  */
  TABLE_LIST *embedding = join_tab->table_ref->embedding;
  if (join_tab->type() == JT_EQ_REF) {
    table_map depends_on = 0;
    uint idx;

    for (uint kp = 0; kp < join_tab->ref().key_parts; kp++)
      depends_on |= join_tab->ref().items[kp]->used_tables();

    Table_map_iterator it(depends_on & ~PSEUDO_TABLE_BITS);
    while ((idx = it.next_bit()) != Table_map_iterator::BITMAP_END) {
      JOIN_TAB *ref_tab = join->map2table[idx];
      if (embedding != ref_tab->table_ref->embedding) return true;
    }
    /* Ok, functionally dependent */
    return false;
  }
  /* Not functionally dependent => need to include*/
  return true;
}


// Source: sql_select.cc
// Lines 1118-1142
