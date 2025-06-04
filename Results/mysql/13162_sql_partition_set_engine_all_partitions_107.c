static void set_engine_all_partitions(partition_info *part_info,
                                      handlerton *engine_type) {
  uint i = 0;
  List_iterator<partition_element> part_it(part_info->partitions);
  do {
    partition_element *part_elem = part_it++;

    part_elem->engine_type = engine_type;
    if (part_info->is_sub_partitioned()) {
      List_iterator<partition_element> sub_it(part_elem->subpartitions);
      uint j = 0;

      do {
        partition_element *sub_elem = sub_it++;

        sub_elem->engine_type = engine_type;
      } while (++j < part_info->num_subparts);


// Source: sql_partition.cc
// Lines 4026-4042
