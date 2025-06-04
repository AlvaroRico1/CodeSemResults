int country_update_column_value(PSI_table_handle *handle, PSI_field *field,
                                unsigned int index) {
  Country_Table_Handle *h = (Country_Table_Handle *)handle;

  char *name = (char *)h->current_row.name;
  unsigned int *name_length = &h->current_row.name_length;
  char *continent_name = (char *)h->current_row.continent_name;
  unsigned int *continent_name_length = &h->current_row.continent_name_length;

  switch (index) {
    case 0: /* COUNTRY_NAME */
      mysql_service_pfs_plugin_table->get_field_char_utf8(field, name,
                                                          name_length);
      break;
    case 1: /* CONTINENT_NAME */
      mysql_service_pfs_plugin_table->get_field_char_utf8(
          field, continent_name, continent_name_length);
      break;
    case 2: /* YEAR */
      mysql_service_pfs_plugin_table->get_field_year(field,
                                                     &h->current_row.year);
      break;
    case 3: /* POPULATION */
      mysql_service_pfs_plugin_table->get_field_bigint(
          field, &h->current_row.population);
      break;
    case 4: /* GROWTH_FACTOR */
      mysql_service_pfs_plugin_table->get_field_double(
          field, &h->current_row.growth_factor);
      break;
    default: /* We should never reach here */
      assert(0);
      break;
  }
  return 0;
}


// Source: pfs_example_country.cc
// Lines 362-397
