int append_file_to_dir(THD *thd, const char **filename_ptr,
                       const char *table_name) {
  char tbbuff[FN_REFLEN];
  char buff[FN_REFLEN];
  char *ptr;
  char *end;

  if (!*filename_ptr) return 0;  // nothing to do

  /* Convert tablename to filename charset so that "/" gets converted
  appropriately */
  size_t tab_len = tablename_to_filename(table_name, tbbuff, sizeof(tbbuff));

  /* Check that the filename is not too long and it's a hard path */
  if (strlen(*filename_ptr) + tab_len >= FN_REFLEN - 1) return ER_PATH_LENGTH;

  if (!test_if_hard_path(*filename_ptr)) return ER_WRONG_VALUE;

  /* Fix is using unix filename format on dos */
  my_stpcpy(buff, *filename_ptr);
  end = convert_dirname(buff, *filename_ptr, NullS);

  ptr = (char *)thd->alloc((size_t)(end - buff) + tab_len + 1);
  if (ptr == nullptr) return ER_OUTOFMEMORY;  // End of memory
  *filename_ptr = ptr;
  strxmov(ptr, buff, tbbuff, NullS);
  return 0;
}


// Source: sql_parse.cc
// Lines 6300-6327
