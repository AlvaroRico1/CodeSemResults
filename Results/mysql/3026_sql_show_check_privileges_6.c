bool Sql_cmd_show_events::check_privileges(THD *thd) {
  const char *db = thd->lex->query_block->db;
  assert(db != nullptr);
  /*
    Nobody has EVENT_ACL for I_S and P_S,
    even with a GRANT ALL to *.*,
    because these schemas have additional ACL restrictions:
    see ACL_internal_schema_registry.

    Yet there are no events in I_S and P_S to hide either,
    so this check voluntarily does not enforce ACL for
    SHOW EVENTS in I_S or P_S,
    to return an empty list instead of an access denied error.

    This is more user friendly, in particular for tools.

    EVENT_ACL is not fine grained enough to differentiate:
    - creating / updating / deleting events
    - viewing existing events
  */
  if (!is_infoschema_db(db) && !is_perfschema_db(db) &&
      check_access(thd, EVENT_ACL, db, nullptr, nullptr, false, false))
    return true;

  return Sql_cmd_show_schema_base::check_privileges(thd);
}


// Source: sql_show.cc
// Lines 472-497
