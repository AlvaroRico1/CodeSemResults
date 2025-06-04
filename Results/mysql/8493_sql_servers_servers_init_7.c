bool servers_init(bool dont_read_servers_table) {
  THD *thd;
  bool return_val = false;
  DBUG_TRACE;

#ifdef HAVE_PSI_INTERFACE
  init_servers_cache_psi_keys();
#endif

  /* init the mutex */
  if (mysql_rwlock_init(key_rwlock_THR_LOCK_servers, &THR_LOCK_servers))
    return true;

  /* initialise our servers cache */
  servers_cache = new collation_unordered_map<std::string, FOREIGN_SERVER *>(
      system_charset_info, key_memory_servers);

  /* Initialize the mem root for data */
  init_sql_alloc(key_memory_servers, &mem, ACL_ALLOC_BLOCK_SIZE, 0);

  if (dont_read_servers_table) goto end;

  /*
    To be able to run this from boot, we allocate a temporary THD
  */
  if (!(thd = new THD)) return true;
  thd->thread_stack = (char *)&thd;
  thd->store_globals();
  /*
    It is safe to call servers_reload() since servers_* arrays and hashes which
    will be freed there are global static objects and thus are initialized
    by zeros at startup.
  */
  return_val = servers_reload(thd);
  delete thd;

end:
  return return_val;
}


// Source: sql_servers.cc
// Lines 190-228
