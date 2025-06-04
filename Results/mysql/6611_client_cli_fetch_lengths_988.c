static void cli_fetch_lengths(ulong *to, MYSQL_ROW column,
                              unsigned int field_count) {
  ulong *prev_length;
  char *start = nullptr;
  MYSQL_ROW end;

  prev_length = nullptr; /* Keep gcc happy */
  for (end = column + field_count + 1; column != end; column++, to++) {
    if (!*column) {
      *to = 0; /* Null */
      continue;
    }
    if (start) /* Found end of prev string */
      *prev_length = (ulong)(*column - start - 1);
    start = *column;
    prev_length = to;
  }
}


// Source: client.cc
// Lines 2281-2298
