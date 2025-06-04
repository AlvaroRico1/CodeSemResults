bool Sql_cmd_alter_table_exchange_partition::exchange_partition(
    THD *thd, TABLE_LIST *table_list, Alter_info *alter_info) {
  TABLE *part_table, *swap_table;
  TABLE_LIST *swap_table_list;
  partition_element *part_elem;
  String *partition_name;
  uint swap_part_id;
  Alter_table_prelocking_strategy alter_prelocking_strategy;
  uint table_counter;
  DBUG_TRACE;
  assert(alter_info->flags & Alter_info::ALTER_EXCHANGE_PARTITION);

  /* Don't allow to exchange with log table */
  swap_table_list = table_list->next_local;
  if (query_logger.check_if_log_table(swap_table_list, false)) {
    my_error(ER_WRONG_USAGE, MYF(0), "PARTITION", "log table");
    return true;
  }

  /*
    Currently no MDL lock that allows both read and write and is upgradeable
    to exclusive, so leave the lock type to TL_WRITE_ALLOW_READ also on the
    partitioned table.

    TODO: add MDL lock that allows both read and write and is upgradable to
    exclusive lock. This would allow to continue using the partitioned table
    also with update/insert/delete while the verification of the swap table
    is running.
  */

  /*
    NOTE: It is not possible to exchange a crashed partition/table since
    we need some info from the engine, which we can only access after open,
    to be able to verify the structure/metadata.
  */
  table_list->mdl_request.set_type(MDL_SHARED_NO_WRITE);
  if (open_tables(thd, &table_list, &table_counter, 0,
                  &alter_prelocking_strategy))
    return true;

  part_table = table_list->table;
  swap_table = swap_table_list->table;

  if (check_exchange_partition(swap_table, part_table)) return true;

  /* set lock pruning on first table */
  partition_name = alter_info->partition_names.head();
  if (table_list->table->part_info->set_named_partition_bitmap(
          partition_name->c_ptr(), partition_name->length()))
    return true;

  if (lock_tables(thd, table_list, table_counter, 0)) return true;

  THD_STAGE_INFO(thd, stage_verifying_table);

  if (!(part_elem = part_table->part_info->get_part_elem(
            partition_name->c_ptr(), &swap_part_id))) {
    my_error(ER_UNKNOWN_PARTITION, MYF(0), partition_name->c_ptr(),
             part_table->alias);
    return true;
  }

  if (swap_part_id == NOT_A_PARTITION_ID) {
    assert(part_table->part_info->is_sub_partitioned());
    my_error(ER_PARTITION_INSTEAD_OF_SUBPARTITION, MYF(0));
    return true;
  }

  if (compare_table_with_partition(thd, swap_table, part_table, part_elem,
                                   swap_part_id))
    return true;

  /* Table and partition has same structure/options */

  if (alter_info->with_validation != Alter_info::ALTER_WITHOUT_VALIDATION) {
    thd_proc_info(thd, "verifying data with partition");

    if (verify_data_with_partition(swap_table, part_table, swap_part_id)) {
      return true;
    }
  }

  /* OK to exchange */

  /*
    Get exclusive mdl lock on both tables, alway the non partitioned table
    first. Remember the tickets for downgrading locks later.
  */
  auto downgrade_mdl_lambda = [thd](MDL_ticket *ticket) {
    if (thd->locked_tables_mode)
      ticket->downgrade_lock(MDL_SHARED_NO_READ_WRITE);
  };
  std::unique_ptr<MDL_ticket, decltype(downgrade_mdl_lambda)>
      swap_tab_downgrade_mdl_guard(swap_table->mdl_ticket,
                                   downgrade_mdl_lambda);
  std::unique_ptr<MDL_ticket, decltype(downgrade_mdl_lambda)>
      part_tab_downgrade_mdl_guard(part_table->mdl_ticket,
                                   downgrade_mdl_lambda);

  /*
    No need to set used_partitions to only propagate
    HA_EXTRA_PREPARE_FOR_RENAME to one part since no built in engine uses
    that flag. And the action would probably be to force close all other
    instances which is what we are doing any way.
  */
  if (wait_while_table_is_used(thd, swap_table, HA_EXTRA_PREPARE_FOR_RENAME) ||
      wait_while_table_is_used(thd, part_table, HA_EXTRA_PREPARE_FOR_RENAME))
    return true;

  DEBUG_SYNC(thd, "swap_partition_after_wait");

  Partition_handler *part_handler;

  if (!(part_handler = part_table->file->get_partition_handler())) {
    my_error(ER_PARTITION_MGMT_ON_NONPARTITIONED, MYF(0));
    return true;
  }

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  dd::Table *part_table_def = nullptr;
  dd::Table *swap_table_def = nullptr;

  if (thd->dd_client()->acquire_for_modification<dd::Table>(
          table_list->db, table_list->table_name, &part_table_def) ||
      thd->dd_client()->acquire_for_modification<dd::Table>(
          swap_table_list->db, swap_table_list->table_name, &swap_table_def))
    return true;

  /* Tables were successfully opened above. */
  assert(part_table_def != nullptr && swap_table_def != nullptr);

  if (part_table_def->options().exists("secondary_engine") ||
      swap_table_def->options().exists("secondary_engine")) {
    /* Exchange operation is not allowed for tables with secondary engine
     * since it's not currently supported by change propagation
     */
    my_error(ER_SECONDARY_ENGINE_DDL, MYF(0));
    return true;
  }

  DEBUG_SYNC(thd, "swap_partition_before_exchange");

  if (dd::sdi::drop_all_for_table(thd, swap_table_def) ||
      dd::sdi::drop_all_for_table(thd, part_table_def)) {
    return true;
  }
  int ha_error = part_handler->exchange_partition(swap_part_id, part_table_def,
                                                  swap_table_def);

  if (ha_error) {
    handlerton *hton = part_table->file->ht;
    part_table->file->print_error(ha_error, MYF(0));
    // Close TABLE instances which marked as old earlier.
    close_all_tables_for_name(thd, swap_table->s, false, nullptr);
    close_all_tables_for_name(thd, part_table->s, false, nullptr);
    /*
      Rollback all possible changes to data-dictionary and SE which
      Partition_handler::exchange_partitions() might have done before
      reporting an error.
      Do this before we downgrade metadata locks.
    */
    (void)trans_rollback_stmt(thd);
    /*
      Full rollback in case we have THD::transaction_rollback_request
      and to synchronize DD state in cache and on disk (as statement
      rollback doesn't clear DD cache of modified uncommitted objects).
    */
    (void)trans_rollback(thd);
    if ((hton->flags & HTON_SUPPORTS_ATOMIC_DDL) && hton->post_ddl)
      hton->post_ddl(thd);
    (void)thd->locked_tables_list.reopen_tables(thd);
    return true;
  } else {
    if (part_table->file->ht->flags & HTON_SUPPORTS_ATOMIC_DDL) {
      handlerton *hton = part_table->file->ht;

      // Close TABLE instances which marked as old earlier.
      close_all_tables_for_name(thd, swap_table->s, false, nullptr);
      close_all_tables_for_name(thd, part_table->s, false, nullptr);

      /*
        Ensure that we call post-DDL hook and re-open tables even
        in case of error.
      */
      auto rollback_post_ddl_reopen_lambda = [hton](THD *thd_arg) {
        /*
          Rollback all possible changes to data-dictionary and SE which
          Partition_handler::exchange_partitions() might have done before
          reporting an error. Do this before we downgrade metadata locks.
        */
        (void)trans_rollback_stmt(thd_arg);
        /*
          Full rollback in case we have THD::transaction_rollback_request
          and to synchronize DD state in cache and on disk (as statement
          rollback doesn't clear DD cache of modified uncommitted objects).
        */
        (void)trans_rollback(thd_arg);
        /*
          Call SE post DDL hook. This handles both rollback and commit cases.
        */
        if (hton->post_ddl) hton->post_ddl(thd_arg);
        (void)thd_arg->locked_tables_list.reopen_tables(thd_arg);
      };

      std::unique_ptr<THD, decltype(rollback_post_ddl_reopen_lambda)>
          rollback_post_ddl_reopen_guard(thd, rollback_post_ddl_reopen_lambda);

      if (thd->dd_client()->update(part_table_def) ||
          thd->dd_client()->update(swap_table_def) ||
          write_bin_log(thd, true, thd->query().str, thd->query().length,
                        true)) {
        return true;
      }

      if (trans_commit_stmt(thd) || trans_commit_implicit(thd)) return true;
    } else {
      /*
        Close TABLE instances which were marked as old earlier and reopen
        tables. Ignore the fact that the statement might fail due to binlog
        write failure.
      */
      close_all_tables_for_name(thd, swap_table->s, false, nullptr);
      close_all_tables_for_name(thd, part_table->s, false, nullptr);
      (void)thd->locked_tables_list.reopen_tables(thd);

      if (write_bin_log(thd, true, thd->query().str, thd->query().length))
        return true;
    }


// Source: sql_partition_admin.cc
// Lines 309-536
