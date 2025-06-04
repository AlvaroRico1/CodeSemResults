  char err_code_buffer[32];
  const char *err_code_ptr = nullptr;
  size_t err_code_length = 0;
  int err_code_num = -1;

  if (j_code > 0)
    err_code_num = j_code;
  else if (j_sym != nullptr) {
    std::string error_symbol_with_terminator(j_sym, j_sym_len);
    err_code_num =
        log_bi->errcode_by_errsymbol(error_symbol_with_terminator.c_str());
  }

  if (err_code_num >= 0) {
    err_code_length = snprintf(err_code_buffer, sizeof(err_code_buffer) - 1,
                               "MY-%06u", err_code_num);
    err_code_ptr = err_code_buffer;
  }

  // convert ISO8601 timestamp to microsecond representation
  ulonglong microseconds = 0;
  if (j_time)
    microseconds = log_bi->parse_iso8601_timestamp(j_time, j_time_len);

  return log_ps->event_add(microseconds, j_thread_id, j_prio, err_code_ptr,
                           err_code_length, j_subsys, j_subsys_len, line_start,
                           line_length);
}


// Source: log_sink_json.cc
// Lines 140-167
