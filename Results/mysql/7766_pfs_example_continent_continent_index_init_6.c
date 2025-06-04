int continent_index_init(PSI_table_handle *handle, unsigned int idx, bool,
                         PSI_index_handle **index) {
  Continent_Table_Handle *h = (Continent_Table_Handle *)handle;

  /* If there are multiple indexes, initialize based on the idx provided */
  switch (idx) {
    case 0: {
      h->index_num = idx;
      Continent_index_by_name *i = &h->m_index;
      /* Initialize first key in index */
      i->m_name.m_name = "NAME";
      i->m_name.m_find_flags = 0;
      i->m_name.m_value_buffer = i->m_name_buffer;
      i->m_name.m_value_buffer_capacity = sizeof(i->m_name_buffer);
      *index = (PSI_index_handle *)i;
    } break;


// Source: pfs_example_continent.cc
// Lines 113-128
