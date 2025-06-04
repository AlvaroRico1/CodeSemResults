void country_close_table(PSI_table_handle *handle) {
  Country_Table_Handle *temp = (Country_Table_Handle *)handle;
  delete temp;
}


// Source: pfs_example_country.cc
// Lines 86-89
