void continent_close_table(PSI_table_handle *handle) {
  Continent_Table_Handle *temp = (Continent_Table_Handle *)handle;
  delete temp;
}


// Source: pfs_example_continent.cc
// Lines 67-70
