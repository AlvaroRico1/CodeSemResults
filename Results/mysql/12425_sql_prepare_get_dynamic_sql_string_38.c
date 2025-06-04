static const char *get_dynamic_sql_string(LEX *lex, size_t *query_len) {
  THD *thd = lex->thd;
  char *query_str = nullptr;

  if (lex->prepared_stmt_code_is_varref) {
    /* This is PREPARE stmt FROM or EXECUTE IMMEDIATE @var. */
    String str;
    const CHARSET_INFO *to_cs = thd->variables.collation_connection;
    bool needs_conversion;
    String *var_value = &str;
    size_t unused;
    size_t len;

    /* Protects thd->user_vars */
    mysql_mutex_lock(&thd->LOCK_thd_data);

    const auto it = thd->user_vars.find(to_string(lex->prepared_stmt_code));

    /*
      Convert @var contents to string in connection character set. Although
      it is known that int/real/NULL value cannot be a valid query we still
      convert it for error messages to be uniform.
    */
    if (it != thd->user_vars.end() && it->second->ptr()) {
      user_var_entry *entry = it->second.get();
      bool is_var_null;
      var_value = entry->val_str(&is_var_null, &str, DECIMAL_NOT_SPECIFIED);

      mysql_mutex_unlock(&thd->LOCK_thd_data);

      /*
        NULL value of variable checked early as entry->value so here
        we can't get NULL in normal conditions
      */
      assert(!is_var_null);
      if (!var_value) goto end;
    } else {
      mysql_mutex_unlock(&thd->LOCK_thd_data);

      /*
        variable absent or equal to NULL, so we need to set variable to
        something reasonable to get a readable error message during parsing
      */
      str.set(STRING_WITH_LEN("NULL"), &my_charset_latin1);
    }

    needs_conversion = String::needs_conversion(
        var_value->length(), var_value->charset(), to_cs, &unused);

    len = (needs_conversion ? var_value->length() * to_cs->mbmaxlen
                            : var_value->length());
    if (!(query_str = (char *)thd->mem_root->Alloc(len + 1))) goto end;

    if (needs_conversion) {
      uint dummy_errors;
      len = copy_and_convert(query_str, len, to_cs, var_value->ptr(),
                             var_value->length(), var_value->charset(),
                             &dummy_errors);
    } else
      memcpy(query_str, var_value->ptr(), var_value->length());
    query_str[len] = '\0';  // Safety (mostly for debug)
    *query_len = len;
  } else {
    query_str = lex->prepared_stmt_code.str;
    *query_len = lex->prepared_stmt_code.length;
  }


// Source: sql_prepare.cc
// Lines 1691-1756
