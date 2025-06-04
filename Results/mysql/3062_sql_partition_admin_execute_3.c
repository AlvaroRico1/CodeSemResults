bool Sql_cmd_alter_table_truncate_partition::execute(THD *thd) {
  int error;
  ulong timeout = thd->variables.lock_wait_timeout;
  TABLE_LIST *first_table = thd->lex->query_block->table_list.first;
  uint table_counter;
  Partition_handler *part_handler = nullptr;
  handlerton *hton;
  DBUG_TRACE;
  assert((m_alter_info->flags & (Alter_info::ALTER_ADMIN_PARTITION |
                                 Alter_info::ALTER_TRUNCATE_PARTITION)) ==
         (Alter_info::ALTER_ADMIN_PARTITION |
          Alter_info::ALTER_TRUNCATE_PARTITION));

  /* Fix the lock types (not the same as ordinary ALTER TABLE). */
  first_table->set_lock({TL_WRITE, THR_DEFAULT});
  first_table->mdl_request.set_type(MDL_EXCLUSIVE);

  /*
    Check table permissions and open it with a exclusive lock.
    Ensure it is a partitioned table and finally, upcast the
    handler and invoke the partition truncate method. Lastly,
    write the statement to the binary log if necessary.
  */

  if (check_one_table_access(thd, DROP_ACL, first_table)) return true;

  if (open_tables(thd, &first_table, &table_counter, 0)) return true;

  if (!first_table->table || first_table->is_view() ||
      !first_table->table->file->ht->partition_flags ||
      !(part_handler = first_table->table->file->get_partition_handler())) {
    my_error(ER_PARTITION_MGMT_ON_NONPARTITIONED, MYF(0));
    return true;
  }

  hton = first_table->table->file->ht;

  /*
    Prune all, but named partitions. SE can use partitions bitmap
    to understand what partitions need to be truncated. This also
    allows to avoid excessive calls to external_lock().
  */
  first_table->partition_names = &m_alter_info->partition_names;
  if (first_table->table->part_info->set_partition_bitmaps(first_table))
    return true;

  /*
    Under locked table modes we still don't have an exclusive lock.
    Hence, upgrade the lock since the handler truncate method mandates
    an exclusive metadata lock. Don't forget to downgrade the lock
    before leaving this method.
  */
  auto downgrade_mdl_lambda = [](MDL_ticket *ticket) {
    ticket->downgrade_lock(MDL_SHARED_NO_READ_WRITE);
  };
  std::unique_ptr<MDL_ticket, decltype(downgrade_mdl_lambda)>
      downgrade_mdl_guard(nullptr, downgrade_mdl_lambda);

  if (thd->locked_tables_mode) {
    MDL_ticket *ticket = first_table->table->mdl_ticket;
    if (thd->mdl_context.upgrade_shared_lock(ticket, MDL_EXCLUSIVE, timeout))
      return true;
    downgrade_mdl_guard.reset(ticket);
  }


// Source: sql_partition_admin.cc
// Lines 584-647
