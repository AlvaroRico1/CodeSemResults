int country_rnd_pos(PSI_table_handle *handle) {
  Country_Table_Handle *h = (Country_Table_Handle *)handle;
  Country_record *record = &country_records_array[h->m_pos.get_index()];

  if (record->m_exist) {
    /* Make the current row from records_array buffer */
    copy_record(&h->current_row, record);
  }

  return 0;
}


// Source: pfs_example_country.cc
// Lines 125-135
