int _db_explain_(CODE_STATE *cs, char *buf, size_t len) {
  char *start = buf, *end = buf + len - 4;

  get_code_state_if_not_set_or_return *buf = 0;

  read_lock_stack(cs);

  op_list_to_buf('d', cs->stack->keywords, DEBUGGING);
  op_int_to_buf('D', cs->stack->delay, 0);
  op_list_to_buf('f', cs->stack->functions, cs->stack->functions);
  op_bool_to_buf('F', cs->stack->flags & FILE_ON);
  op_bool_to_buf('i', cs->stack->flags & PID_ON);
  op_list_to_buf('g', cs->stack->p_functions, PROFILING);
  op_bool_to_buf('L', cs->stack->flags & LINE_ON);
  op_bool_to_buf('n', cs->stack->flags & DEPTH_ON);
  op_bool_to_buf('N', cs->stack->flags & NUMBER_ON);
  op_str_to_buf(((cs->stack->flags & FLUSH_ON_WRITE ? 0 : 32) |
                 (cs->stack->flags & OPEN_APPEND ? 'A' : 'O')),
                cs->stack->name, cs->stack->out_file != stderr);
  op_list_to_buf('p', cs->stack->processes, cs->stack->processes);
  op_bool_to_buf('P', cs->stack->flags & PROCESS_ON);
  op_bool_to_buf('r', cs->stack->sub_level != 0);
  op_intf_to_buf('t', cs->stack->maxdepth, MAXDEPTH, TRACING);
  op_bool_to_buf('T', cs->stack->flags & TIMESTAMP_ON);

  unlock_stack(cs);

  *buf = '\0';
  return 0;

overflow:
  *end++ = '.';
  *end++ = '.';
  *end++ = '.';
  *end = '\0';

  unlock_stack(cs);
  return 1;
}


// Source: dbug.cc
// Lines 1018-1056
