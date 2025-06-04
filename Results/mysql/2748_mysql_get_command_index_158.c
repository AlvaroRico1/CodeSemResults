inline int get_command_index(char cmd_char) {
  /*
    All client-specific commands are in the first part of commands array
    and have a function to implement it.
  */
  for (uint i = 0; *commands[i].func != nullptr; i++)
    if (commands[i].cmd_char == cmd_char) return i;
  return -1;
}


// Source: mysql.cc
// Lines 1209-1217
