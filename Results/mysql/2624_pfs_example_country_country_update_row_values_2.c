int country_update_row_values(PSI_table_handle *handle) {
  int result = 0;
  Country_Table_Handle *h = (Country_Table_Handle *)handle;

  Country_record *cur = &country_records_array[h->m_pos.get_index()];

  assert(cur->m_exist == true);

  native_mutex_lock(&LOCK_country_records_array);
  if (is_duplicate(&h->current_row, h->m_pos.get_index()))
    result = PFS_HA_ERR_FOUND_DUPP_KEY;
  else
    copy_record(cur, &h->current_row);
  native_mutex_unlock(&LOCK_country_records_array);

  return result;
}


// Source: pfs_example_country.cc
// Lines 344-360
