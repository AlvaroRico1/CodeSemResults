bool prepare_index_and_data_dir_path(THD *thd, const char **data_file_name,
                                     const char **index_file_name,
                                     const char *table_name) {
  int ret_val;
  const char *file_name;
  const char *directory_type;

  /*
    If a data directory path is passed, check if the path exists and append
    table_name to it.
  */
  if (data_file_name &&
      (ret_val = append_file_to_dir(thd, data_file_name, table_name))) {
    file_name = *data_file_name;
    directory_type = "DATA DIRECTORY";
    goto err;
  }

  /*
    If an index directory path is passed, check if the path exists and append
    table_name to it.
  */
  if (index_file_name &&
      (ret_val = append_file_to_dir(thd, index_file_name, table_name))) {
    file_name = *index_file_name;
    directory_type = "INDEX DIRECTORY";
    goto err;
  }

  return false;
err:
  if (ret_val == ER_PATH_LENGTH)
    my_error(ER_PATH_LENGTH, MYF(0), directory_type);
  if (ret_val == ER_WRONG_VALUE)
    my_error(ER_WRONG_VALUE, MYF(0), "path", file_name);
  return true;
}


// Source: sql_parse.cc
// Lines 6260-6296
