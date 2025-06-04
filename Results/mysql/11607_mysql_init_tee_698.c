static void init_tee(const char *file_name) {
  FILE *new_outfile;
  if (opt_outfile) end_tee();
  if (!(new_outfile = my_fopen(file_name, O_APPEND | O_WRONLY, MYF(MY_WME)))) {
    tee_fprintf(stdout, "Error logging to file '%s'\n", file_name);
    return;
  }
  OUTFILE = new_outfile;
  strmake(outfile, file_name, FN_REFLEN - 1);
  tee_fprintf(stdout, "Logging to file '%s'\n", file_name);
  opt_outfile = true;
  return;
}


// Source: mysql.cc
// Lines 3450-3462
