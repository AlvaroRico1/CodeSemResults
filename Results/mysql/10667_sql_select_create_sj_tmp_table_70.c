SJ_TMP_TABLE *create_sj_tmp_table(THD *thd, JOIN *join,
                                  SJ_TMP_TABLE_TAB *first_tab,
                                  SJ_TMP_TABLE_TAB *last_tab) {
  uint jt_rowid_offset =
      0;                  // # tuple bytes are already occupied (w/o NULL bytes)
  uint jt_null_bits = 0;  // # null bits in tuple bytes
  for (SJ_TMP_TABLE_TAB *tab = first_tab; tab != last_tab; ++tab) {
    QEP_TAB *qep_tab = tab->qep_tab;
    tab->rowid_offset = jt_rowid_offset;
    jt_rowid_offset += qep_tab->table()->file->ref_length;
    if (qep_tab->table()->is_nullable()) {
      tab->null_byte = jt_null_bits / 8;
      tab->null_bit = jt_null_bits++;
    }
    qep_tab->table()->prepare_for_position();
  }

  SJ_TMP_TABLE *sjtbl;
  if (jt_rowid_offset) /* Temptable has at least one rowid */
  {
    sjtbl = new (thd->mem_root) SJ_TMP_TABLE;
    if (sjtbl == nullptr) return nullptr;
    sjtbl->tabs =
        thd->mem_root->ArrayAlloc<SJ_TMP_TABLE_TAB>(last_tab - first_tab);
    if (sjtbl->tabs == nullptr) return nullptr;
    sjtbl->tabs_end = std::uninitialized_copy(first_tab, last_tab, sjtbl->tabs);
    sjtbl->is_confluent = false;
    sjtbl->rowid_len = jt_rowid_offset;
    sjtbl->null_bits = jt_null_bits;
    sjtbl->null_bytes = (jt_null_bits + 7) / 8;
    sjtbl->tmp_table = create_duplicate_weedout_tmp_table(
        thd, sjtbl->rowid_len + sjtbl->null_bytes, sjtbl);
    if (sjtbl->tmp_table == nullptr) return nullptr;
    if (sjtbl->tmp_table->hash_field)
      sjtbl->tmp_table->file->ha_index_init(0, false);
    join->sj_tmp_tables.push_back(sjtbl->tmp_table);
  } else {
    /*
      This is confluent case where the entire subquery predicate does
      not depend on anything at all, ie this is
        WHERE const IN (uncorrelated select)
    */
    if (!(sjtbl = new (thd->mem_root) SJ_TMP_TABLE))
      return nullptr; /* purecov: inspected */
    sjtbl->tmp_table = nullptr;
    sjtbl->is_confluent = true;
    sjtbl->have_confluent_row = false;
  }
  return sjtbl;
}


// Source: sql_select.cc
// Lines 1144-1193
