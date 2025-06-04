static COMMANDS *find_command(char cmd_char) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("cmd_char: %d", cmd_char));

  int index = -1;

  /*
    In binary-mode, we disallow all mysql commands except '\C'
    and DELIMITER.
  */
  if (real_binary_mode) {
    if (cmd_char == 'C') index = charset_index;
  } else
    index = get_command_index(cmd_char);

  if (index >= 0) {
    DBUG_PRINT("exit", ("found command: %s", commands[index].name));
    return &commands[index];
  } else
    return (COMMANDS *)nullptr;
}


// Source: mysql.cc
// Lines 2364-2384
