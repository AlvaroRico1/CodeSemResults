int country_delete_row_values(PSI_table_handle *handle) {
  Country_Table_Handle *h = (Country_Table_Handle *)handle;

  Country_record *cur = &country_records_array[h->m_pos.get_index()];

  assert(cur->m_exist == true);

  native_mutex_lock(&LOCK_country_records_array);
  cur->m_exist = false;
  country_rows_in_table--;
  native_mutex_unlock(&LOCK_country_records_array);

  return 0;
}


// Source: pfs_example_country.cc
// Lines 400-413
