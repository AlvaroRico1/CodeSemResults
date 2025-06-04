static int com_charset(String *buffer MY_ATTRIBUTE((unused)), char *line) {
  char buff[256], *param;
  const CHARSET_INFO *new_cs;
  strmake(buff, line, sizeof(buff) - 1);
  param = get_arg(buff, false);
  if (!param || !*param) {
    return put_info("Usage: \\C charset_name | charset charset_name",
                    INFO_ERROR, 0);
  }
  new_cs = get_charset_by_csname(param, MY_CS_PRIMARY, MYF(MY_WME));
  if (new_cs) {
    charset_info = new_cs;
    mysql_set_character_set(&mysql, replace_utf8_utf8mb3(charset_info->csname));
    default_charset = replace_utf8_utf8mb3(charset_info->csname);
    put_info("Charset changed", INFO_INFO);
  } else
    put_info("Charset is not found", INFO_INFO);
  return 0;
}


// Source: mysql.cc
// Lines 3263-3281
