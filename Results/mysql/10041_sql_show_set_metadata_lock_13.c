bool Sql_cmd_show_schema_base::set_metadata_lock(THD *thd) {
  LEX_STRING lex_str_db;
  LEX *lex = thd->lex;
  if (lex_string_strmake(thd->mem_root, &lex_str_db, lex->query_block->db,
                         strlen(lex->query_block->db)))
    return true;

  // Acquire IX MDL lock on schema name.
  MDL_request mdl_request;
  MDL_REQUEST_INIT(&mdl_request, MDL_key::SCHEMA, lex_str_db.str, "",
                   MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
  if (thd->mdl_context.acquire_lock(&mdl_request,
                                    thd->variables.lock_wait_timeout))
    return true;
  return false;
}


// Source: sql_show.cc
// Lines 208-223
