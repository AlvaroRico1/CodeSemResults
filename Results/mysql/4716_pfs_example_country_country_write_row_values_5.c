int country_write_row_values(PSI_table_handle *handle) {
  Country_Table_Handle *h = (Country_Table_Handle *)handle;

  native_mutex_lock(&LOCK_country_records_array);

  /* If there is no more space for inserting a record, return */
  if (country_rows_in_table >= COUNTRY_MAX_ROWS) {
    native_mutex_unlock(&LOCK_country_records_array);
    return PFS_HA_ERR_RECORD_FILE_FULL;
  }

  h->current_row.m_exist = true;

  if (is_duplicate(&h->current_row, -1)) {
    native_mutex_unlock(&LOCK_country_records_array);
    return PFS_HA_ERR_FOUND_DUPP_KEY;
  }

  copy_record(&country_records_array[country_next_available_index],
              &h->current_row);
  country_rows_in_table++;

  /* set next available index */
  if (country_rows_in_table < COUNTRY_MAX_ROWS) {
    int i = (country_next_available_index + 1) % COUNTRY_MAX_ROWS;
    int itr_count = 0;
    while (itr_count < COUNTRY_MAX_ROWS) {
      if (country_records_array[i].m_exist == false) {
        country_next_available_index = i;
        break;
      }
      i = (i + 1) % COUNTRY_MAX_ROWS;
      itr_count++;
    }
  }

  native_mutex_unlock(&LOCK_country_records_array);

  return 0;
}


// Source: pfs_example_country.cc
// Lines 263-302
