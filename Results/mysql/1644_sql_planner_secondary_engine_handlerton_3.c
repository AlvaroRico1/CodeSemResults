static const handlerton *secondary_engine_handlerton(const THD *thd) {
  const Sql_cmd *sql_cmd = thd->lex->m_sql_cmd;
  if (sql_cmd == nullptr) return nullptr;
  return sql_cmd->secondary_engine();
}


// Source: sql_planner.cc
// Lines 2429-2433
