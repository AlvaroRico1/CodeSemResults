int continent_index_read(PSI_index_handle *index, PSI_key_reader *reader,
                         unsigned int idx, int find_flag) {
  switch (idx) {
    case 0: {
      Continent_index_by_name *i = (Continent_index_by_name *)index;
      /* Read all keys on index one by one */
      mysql_service_pfs_plugin_table->read_key_string(reader, &i->m_name,
                                                      find_flag);
    } break;


// Source: pfs_example_continent.cc
// Lines 138-146
