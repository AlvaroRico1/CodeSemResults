static void print_as_hex(FILE *output_file, const char *str, ulong len,
                         ulong total_bytes_to_send) {
  const char *ptr = str, *end = ptr + len;
  ulong i;

  if (len > 0) {
    fprintf(output_file, "0x");
    for (; ptr < end; ptr++)
      fprintf(output_file, "%02X",
              *(static_cast<const uchar *>(static_cast<const void *>(ptr))));
    /* Printed string length: two chars "0x" + two chars for each byte. */
    i = 2 + len * 2;
  } else {
    i = fprintf(output_file, "NULL");
  }
  for (; i < total_bytes_to_send; i++)
    tee_putc(static_cast<int>(' '), output_file);
}


// Source: mysql.cc
// Lines 3558-3575
