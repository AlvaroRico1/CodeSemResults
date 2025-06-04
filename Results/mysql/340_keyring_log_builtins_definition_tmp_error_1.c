  st_error *tmp_error;
  const char *errtxt{"Unknown error"};

  tmp_error = &global_error_names[0];

  while (tmp_error->name != nullptr) {
    if (tmp_error->errnr == mysql_errcode) {
      errtxt = tmp_error->text;
      break;
    }
    tmp_error++;
  }
  return errtxt;
}


// Source: keyring_log_builtins_definition.cc
// Lines 285-298
