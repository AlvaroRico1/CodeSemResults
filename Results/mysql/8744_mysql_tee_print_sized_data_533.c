static void tee_print_sized_data(const char *data, unsigned int data_length,
                                 unsigned int total_bytes_to_send,
                                 bool right_justified) {
  /*
    For '\0's print ASCII spaces instead, as '\0' is eaten by (at
    least my) console driver, and that messes up the pretty table
    grid.  (The \0 is also the reason we can't use fprintf() .)
  */
  unsigned int i;

  if (right_justified)
    for (i = data_length; i < total_bytes_to_send; i++)
      tee_putc((int)' ', PAGER);

  tee_write(PAGER, data, data_length, MY_PRINT_SPS_0 | MY_PRINT_MB);

  if (!right_justified)
    for (i = data_length; i < total_bytes_to_send; i++)
      tee_putc((int)' ', PAGER);
}


// Source: mysql.cc
// Lines 3744-3763
