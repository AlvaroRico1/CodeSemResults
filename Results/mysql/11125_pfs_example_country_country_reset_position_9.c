void country_reset_position(PSI_table_handle *handle) {
  Country_Table_Handle *h = (Country_Table_Handle *)handle;
  h->m_pos.reset();
  h->m_next_pos.reset();
  return;
}


// Source: pfs_example_country.cc
// Lines 220-225
