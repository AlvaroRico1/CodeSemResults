int continent_read_column_value(PSI_table_handle *handle, PSI_field *field,
                                unsigned int index) {
  Continent_Table_Handle *h = (Continent_Table_Handle *)handle;

  switch (index) {
    case 0: /* NAME */
      mysql_service_pfs_plugin_table->set_field_char_utf8(
          field, h->current_row.name, h->current_row.name_length);
      break;
    default: /* We should never reach here */
      assert(0);
      break;
  }

  return 0;
}


// Source: pfs_example_continent.cc
// Lines 193-208
