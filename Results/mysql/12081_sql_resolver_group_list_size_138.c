int Query_block::group_list_size() const {
  int size = 0;
  for (ORDER *group = group_list.first; group; group = group->next) {
    ++size;
  }
  return size;
}


// Source: sql_resolver.cc
// Lines 4638-4644
