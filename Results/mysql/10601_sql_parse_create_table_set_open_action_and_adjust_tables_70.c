void create_table_set_open_action_and_adjust_tables(LEX *lex) {
  TABLE_LIST *create_table = lex->query_tables;

  if (lex->create_info->options & HA_LEX_CREATE_TMP_TABLE)
    create_table->open_type = OT_TEMPORARY_ONLY;
  else
    create_table->open_type = OT_BASE_ONLY;

  if (lex->query_block->fields.empty()) {
    /*
      Avoid opening and locking target table for ordinary CREATE TABLE
      or CREATE TABLE LIKE for write (unlike in CREATE ... SELECT we
      won't do any insertions in it anyway). Not doing this causes
      problems when running CREATE TABLE IF NOT EXISTS for already
      existing log table.
    */
    create_table->set_lock({TL_READ, THR_DEFAULT});
  }
}


// Source: sql_parse.cc
// Lines 6394-6412
