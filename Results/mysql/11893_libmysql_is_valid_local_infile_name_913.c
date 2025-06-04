static bool is_valid_local_infile_name(st_mysql_options *options,
                                       const char *net_filename) {
  char buff1[FN_REFLEN], buff2[FN_REFLEN];

  ENSURE_EXTENSIONS_PRESENT(options);

  // null load_data_dir means no exceptions (compatibility)
  if (options->extension->load_data_dir == nullptr) return false;

  // make fully qualified name
  if (my_realpath(buff1, net_filename, 0)) return false;

  // with uniform directory separators
  convert_dirname(buff2, buff1, NullS);

  /* if the name supplied starts with load_data_dir accept it */
  int ret = strncmp(options->extension->load_data_dir, buff2,
                    strlen(options->extension->load_data_dir));
  return ret == 0;
}


// Source: libmysql.cc
// Lines 383-402
