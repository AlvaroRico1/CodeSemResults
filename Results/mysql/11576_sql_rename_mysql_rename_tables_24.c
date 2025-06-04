bool mysql_rename_tables(THD *thd, TABLE_LIST *table_list) {
  TABLE_LIST *ren_table = nullptr;
  DBUG_TRACE;

  mysql_ha_rm_tables(thd, table_list);

  /*
    The below Auto_releaser allows to keep uncommitted versions of data-
    dictionary objects cached in the Dictionary_client for the whole duration
    of the statement.
  */
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  if (query_logger.is_log_table_enabled(QUERY_LOG_GENERAL) ||
      query_logger.is_log_table_enabled(QUERY_LOG_SLOW)) {
    int to_table;
    const char *rename_log_table[2] = {nullptr, nullptr};

    /*
      Rules for rename of a log table:

      IF   1. Log tables are enabled
      AND  2. Rename operates on the log table and nothing is being
              renamed to the log table.
      DO   3. Throw an error message.
      ELSE 4. Perform rename.
    */

    for (to_table = 0, ren_table = table_list; ren_table;
         to_table = 1 - to_table, ren_table = ren_table->next_local) {
      int log_table_rename = 0;

      if ((log_table_rename =
               query_logger.check_if_log_table(ren_table, true))) {
        /*
          as we use log_table_rename as an array index, we need it to start
          with 0, while QUERY_LOG_SLOW == 1 and QUERY_LOG_GENERAL == 2.
          So, we shift the value to start with 0;
        */
        log_table_rename--;
        if (rename_log_table[log_table_rename]) {
          if (to_table)
            rename_log_table[log_table_rename] = nullptr;
          else {
            /*
              Two renames of "log_table TO" w/o rename "TO log_table" in
              between.
            */
            my_error(ER_CANT_RENAME_LOG_TABLE, MYF(0), ren_table->table_name,
                     ren_table->table_name);
            return true;
          }
        } else {
          if (to_table) {
            /*
              Attempt to rename a table TO log_table w/o renaming
              log_table TO some table.
            */
            my_error(ER_CANT_RENAME_LOG_TABLE, MYF(0), ren_table->table_name,
                     ren_table->table_name);
            return true;
          } else {
            /* save the name of the log table to report an error */
            rename_log_table[log_table_rename] = ren_table->table_name;
          }
        }
      }
    }
    if (rename_log_table[0] || rename_log_table[1]) {
      if (rename_log_table[0])
        my_error(ER_CANT_RENAME_LOG_TABLE, MYF(0), rename_log_table[0],
                 rename_log_table[0]);
      else
        my_error(ER_CANT_RENAME_LOG_TABLE, MYF(0), rename_log_table[1],
                 rename_log_table[1]);
      return true;
    }
  }

  /*
    Array in which pointers to MDL requests for acquired schema locks are
    stored. Each schema can be present in this array only once.
  */
  Prealloced_array<MDL_request *, 1> schema_reqs(PSI_INSTRUMENT_ME);

  if (thd->locked_tables_mode) {
    /*
      LOCK TABLES case.

      Check that tables to be renamed are locked for WRITE. Take into
      account that name of table to be renamed might be result of some
      previous step in multi-step RENAME TABLES.

      In theory, we could disregard whether they locked or not and just try
      to acquire exclusive metadata locks on them, but this is too deadlock
      prone.

      Most probably, there is no tables which correspond to target table
      names, so similar check doesn't make sense for them.

      In theory, we can reduce chance of MDL deadlocks by also checking at
      this stage that all child and parent tables for FKs in which tables
      to be renamed participate are locked for WRITE (as we will have to
      acquire to exclusive MDLs on these tables later).
      But this is, probably, too severe restriction which will make
      RENAMES TABLES under LOCK TABLES hard to use in 3rd-party online
      ALTER TABLE tools.
    */
    malloc_unordered_set<TABLE_LIST *, table_list_hash, table_list_equal>
        new_names(PSI_INSTRUMENT_ME);

    TABLE_LIST *new_table;
    for (ren_table = table_list; ren_table; ren_table = new_table->next_local) {
      new_table = ren_table->next_local;

      auto new_name_it = new_names.find(ren_table);
      if (new_name_it == new_names.end()) {
        if (check_if_owns_upgradable_mdl(thd, ren_table->db,
                                         ren_table->table_name))
          return true;
      } else {
        new_names.erase(new_name_it);
      }
      new_names.insert(new_table);
    }

    /*
      Now proceed to acquiring exclusive metadata locks on both source and
      target table names as well as necessary schema, global and backup locks.
      Since we already have SNRW locks on source table names, we, in fact, are
      upgrading locks for them.
    */
  }

  if (lock_table_names(thd, table_list, nullptr,
                       thd->variables.lock_wait_timeout, 0, &schema_reqs) ||
      lock_trigger_names(thd, table_list))
    return true;

  const dd::Table *table_def = nullptr;
  for (TABLE_LIST *table = table_list; table && table->next_local;
       table = table->next_local) {
    if (thd->dd_client()->acquire(table->db, table->table_name, &table_def)) {
      return true;
    }
    if (table_def && table_def->hidden() == dd::Abstract_table::HT_HIDDEN_SE) {
      my_error(ER_NO_SUCH_TABLE, MYF(0), table->db, table->table_name);
      return true;
    }
  }

  for (ren_table = table_list; ren_table; ren_table = ren_table->next_local) {
    if (thd->locked_tables_mode)
      close_all_tables_for_name(thd, ren_table->db, ren_table->table_name,
                                false);
    else
      tdc_remove_table(thd, TDC_RT_REMOVE_ALL, ren_table->db,
                       ren_table->table_name, false);
  }
  bool error = false;
  bool int_commit_done = false;
  /*
    Indicates whether we managed fully revert non-atomic RENAME TABLES
    after the failure.
  */
  bool int_commit_full_revert = false;
  std::set<handlerton *> post_ddl_htons;
  Foreign_key_parents_invalidator fk_invalidator;
  /*
    An exclusive lock on table names is satisfactory to ensure
    no other thread accesses this table.
  */
  if ((ren_table = rename_tables(thd, table_list, &int_commit_done,
                                 &post_ddl_htons, &fk_invalidator))) {
    /* Rename didn't succeed;  rename back the tables in reverse order */
    TABLE_LIST *table;

    if (int_commit_done) {
      /* Reverse the table list */
      table_list = reverse_table_list(table_list);

      /* Find the last renamed table */
      for (table = table_list; table->next_local != ren_table;
           table = table->next_local->next_local)
        ;
      table = table->next_local->next_local;  // Skip error table

      /*
        Revert to old names. In 5.7 we have ignored most of errors occurring
        in the process. However, this looks like a risky idea -- by ignoring
        errors we are likely to end up in some awkward state and not going to
        restore status quo ante.

        So starting from 8.0 we chose to abort reversal on the first failure.
        We will still end up in some awkward case in this case but at least
        no additional damage will be done. Note that since InnoDB tables are
        new default and this engine supports atomic DDL, non-atomic RENAME
        TABLES, which this code deals with, is not the main use case anyway.
      */
      int_commit_full_revert = !rename_tables(thd, table, &int_commit_done,
                                              &post_ddl_htons, &fk_invalidator);

      /* Revert the table list (for prepared statements) */
      table_list = reverse_table_list(table_list);
    }

    error = true;
  }

  if (!error) {
    error = write_bin_log(thd, true, thd->query().str, thd->query().length,
                          !int_commit_done);
  }

  if (!error) {
    Uncommitted_tables_guard uncommitted_tables(thd);

    for (ren_table = table_list; ren_table;
         ren_table = ren_table->next_local->next_local) {
      TABLE_LIST *new_table = ren_table->next_local;
      assert(new_table);

      uncommitted_tables.add_table(ren_table);
      uncommitted_tables.add_table(new_table);

      if ((error = update_referencing_views_metadata(
               thd, ren_table, new_table->db, new_table->table_name,
               int_commit_done, &uncommitted_tables)))
        break;
    }
  }

  if (!error && !int_commit_done) {
    error = (trans_commit_stmt(thd) || trans_commit_implicit(thd));

    if (!error) {
      /*
        Don't try to invalidate foreign key parents on error,
        as we might miss necessary locks on them.
      */
      fk_invalidator.invalidate(thd);
    }
  }

  if (error) {
    trans_rollback_stmt(thd);
    /*
      Full rollback in case we have THD::transaction_rollback_request
      and to synchronize DD state in cache and on disk (as statement
      rollback doesn't clear DD cache of modified uncommitted objects).
    */
    trans_rollback(thd);
  }

  for (handlerton *hton : post_ddl_htons) hton->post_ddl(thd);

  if (thd->locked_tables_mode) {
    if (!error) {
      /*
        Adjust locked tables list and reopen tables under new names.
        Also calculate sets of metadata locks to release (on old table
        names) and to keep until UNLOCK TABLES (on new table names).

        In addition to keeping locks on tables we also do the same for
        schemas in order to keep set of metadata locks consistent with
        one acquired by LOCK TABLES. We don't release locks on old table
        schemas as it is non-trivial to figure out which locks can be
        released.

        Tablespaces do not need special handling though, as metadata locks
        on them are acquired at LOCK TABLES time and are unaffected by
        RENAME TABLES.
      */
      malloc_unordered_set<TABLE_LIST *, table_list_hash, table_list_equal>
          to_release(PSI_INSTRUMENT_ME), to_keep(PSI_INSTRUMENT_ME);
      TABLE_LIST *new_table;
      for (ren_table = table_list; ren_table;
           ren_table = new_table->next_local) {
        new_table = ren_table->next_local;
        thd->locked_tables_list.rename_locked_table(
            ren_table, new_table->db, new_table->table_name,
            new_table->mdl_request.ticket);
        to_release.insert(ren_table);
        to_keep.erase(ren_table);
        to_keep.insert(new_table);
        to_release.erase(new_table);
      }

      error = thd->locked_tables_list.reopen_tables(thd);

      for (TABLE_LIST *t : to_release) {
        // Also releases locks with EXPLICIT duration for the same name.
        thd->mdl_context.release_all_locks_for_name(t->mdl_request.ticket);
      }

      for (TABLE_LIST *t : to_keep) {
        thd->mdl_context.set_lock_duration(t->mdl_request.ticket, MDL_EXPLICIT);
        t->mdl_request.ticket->downgrade_lock(MDL_SHARED_NO_READ_WRITE);
        find_and_set_explicit_duration_for_schema_mdl(thd, t, &schema_reqs);
      }
    } else if (!int_commit_done || int_commit_full_revert) {
      /*
        Error happened but all (actually not quite all, see below) changes
        were reverted. We just need to reopen tables.

        Since changes were reverted no additional metadata locks need to
        be kept after statement end. All additional locks acquired by
        this statement will be released automatically at its end, since
        they have transactional duration.

        In case of non-atomic RENAME TABLE previously orphan foreign keys
        which got new parents will keep these parents after reversal, but
        this is not important in this context.
      */
      thd->locked_tables_list.reopen_tables(thd);
    } else {
      /*
        Error happened and we failed to revert all changes. We simply close
        all tables involved.
      */
      thd->locked_tables_list.unlink_all_closed_tables(thd, nullptr, 0);
      /*
        We need to keep metadata locks on both old and new table names
        to avoid breaking foreign key invariants for LOCK TABLES.
        So we set duration of locks on new names to explicit and downgrade
        them from X to SNRW metadata locks. Also keep locks for new schemas.

        Prune list of duplicates first as setting explicit duration for the
        same MDL ticket twice is disallowed.
      */
      malloc_unordered_set<TABLE_LIST *, table_list_hash, table_list_equal>
          to_keep(PSI_INSTRUMENT_ME);
      TABLE_LIST *new_table;
      for (ren_table = table_list; ren_table;
           ren_table = new_table->next_local) {
        new_table = ren_table->next_local;
        to_keep.insert(new_table);
      }
      for (TABLE_LIST *t : to_keep) {
        thd->mdl_context.set_lock_duration(t->mdl_request.ticket, MDL_EXPLICIT);
        t->mdl_request.ticket->downgrade_lock(MDL_SHARED_NO_READ_WRITE);
        find_and_set_explicit_duration_for_schema_mdl(thd, t, &schema_reqs);
      }
    }


// Source: sql_rename.cc
// Lines 155-498
