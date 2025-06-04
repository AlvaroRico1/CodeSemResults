int continent_rnd_next(PSI_table_handle *handle) {
  Continent_Table_Handle *h = (Continent_Table_Handle *)handle;

  for (h->m_pos.set_at(&h->m_next_pos); h->m_pos.has_more(); h->m_pos.next()) {
    Continent_record *record = &continent_records_array[h->m_pos.get_index()];

    if (record->m_exist) {
      /* Make the current row from records_array buffer */
      copy_record(&h->current_row, record);
      h->m_next_pos.set_after(&h->m_pos);
      return 0;
    }
  }

  return PFS_HA_ERR_END_OF_FILE;
}


// Source: pfs_example_continent.cc
// Lines 80-95
