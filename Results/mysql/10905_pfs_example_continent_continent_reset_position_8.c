void continent_reset_position(PSI_table_handle *handle) {
  Continent_Table_Handle *h = (Continent_Table_Handle *)handle;
  h->m_pos.reset();
  h->m_next_pos.reset();
  return;
}


// Source: pfs_example_continent.cc
// Lines 185-190
