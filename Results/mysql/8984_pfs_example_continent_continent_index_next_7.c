int continent_index_next(PSI_table_handle *handle) {
  Continent_Table_Handle *h = (Continent_Table_Handle *)handle;
  Continent_index *i = nullptr;

  switch (h->index_num) {
    case 0:
      i = (Continent_index_by_name *)&h->m_index;
      break;
    default:
      assert(0);
      break;
  }

  for (h->m_pos.set_at(&h->m_next_pos); h->m_pos.has_more(); h->m_pos.next()) {
    Continent_record *record = &continent_records_array[h->m_pos.get_index()];

    if (record->m_exist) {
      if (i->match(record)) {
        copy_record(&h->current_row, record);
        h->m_next_pos.set_after(&h->m_pos);
        return 0;
      }
    }
  }

  return PFS_HA_ERR_END_OF_FILE;
}


// Source: pfs_example_continent.cc
// Lines 156-182
