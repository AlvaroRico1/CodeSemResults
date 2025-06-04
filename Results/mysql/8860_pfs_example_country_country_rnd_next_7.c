int country_rnd_next(PSI_table_handle *handle) {
  Country_Table_Handle *h = (Country_Table_Handle *)handle;

  for (h->m_pos.set_at(&h->m_next_pos); h->m_pos.has_more(); h->m_pos.next()) {
    Country_record *record = &country_records_array[h->m_pos.get_index()];

    if (record->m_exist) {
      /* Make the current row from records_array buffer */
      copy_record(&h->current_row, record);
      h->m_next_pos.set_after(&h->m_pos);
      return 0;
    }
  }

  return PFS_HA_ERR_END_OF_FILE;
}


// Source: pfs_example_country.cc
// Lines 105-120
