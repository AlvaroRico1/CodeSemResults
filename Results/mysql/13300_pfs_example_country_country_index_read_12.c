int country_index_read(PSI_index_handle *index, PSI_key_reader *reader,
                       unsigned int idx, int find_flag) {
  switch (idx) {
    case 0: {
      Country_index_by_name *i = (Country_index_by_name *)index;
      /* Read all keys on index one by one */
      mysql_service_pfs_plugin_table->read_key_string(
          reader, &i->m_country_name, find_flag);
      mysql_service_pfs_plugin_table->read_key_string(
          reader, &i->m_continent_name, find_flag);
    } break;


// Source: pfs_example_country.cc
// Lines 171-181
