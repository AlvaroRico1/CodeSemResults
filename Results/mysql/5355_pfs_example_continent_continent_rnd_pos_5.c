int continent_rnd_pos(PSI_table_handle *handle) {
  Continent_Table_Handle *h = (Continent_Table_Handle *)handle;
  Continent_record *record = &continent_records_array[h->m_pos.get_index()];

  if (record->m_exist) {
    /* Make the current row from records_array buffer */
    copy_record(&h->current_row, record);
  }

  return 0;
}


// Source: pfs_example_continent.cc
// Lines 100-110
