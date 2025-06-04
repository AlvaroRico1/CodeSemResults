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


// Source: sql_select.cc
// Lines 1144-1159
