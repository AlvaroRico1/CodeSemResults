void prune_partition_set(const TABLE *table, part_id_range *part_spec) {
  int last_partition = -1;
  uint i = part_spec->start_part;
  partition_info *part_info = table->part_info;
  DBUG_TRACE;

  if (i)
    i = bitmap_get_next_set(&part_info->read_partitions, i - 1);
  else
    i = bitmap_get_first_set(&part_info->read_partitions);

  part_spec->start_part = i;

  /* TODO: Only check next bit, no need to prune end if >= 2 partitions. */
  for (; i <= part_spec->end_part;
       i = bitmap_get_next_set(&part_info->read_partitions, i)) {
    DBUG_PRINT("info", ("Partition %d is set", i));
    if (last_partition == -1)
      /* First partition found in set and pruned bitmap */
      part_spec->start_part = i;
    last_partition = i;
  }
  if (last_partition == -1) /* No partition found in pruned bitmap */
    part_spec->start_part = part_spec->end_part + 1;
  else  // if (last_partition != -1)
    part_spec->end_part = last_partition;
}


// Source: sql_partition.cc
// Lines 3622-3648
