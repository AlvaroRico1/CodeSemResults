static int com_ego(String *buffer, char *line) {
  int result;
  bool oldvertical = vertical;
  vertical = true;
  result = com_go(buffer, line);
  vertical = oldvertical;
  return result;
}


// Source: mysql.cc
// Lines 3471-3478
