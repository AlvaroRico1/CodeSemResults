bool my_parse_charset_xml(MY_CHARSET_LOADER *loader, const char *buf,
                          size_t len) {
  MY_XML_PARSER p;
  struct my_cs_file_info info;
  bool rc;

  my_charset_file_init(&info);
  my_xml_parser_create(&p);
  my_xml_set_enter_handler(&p, cs_enter);
  my_xml_set_value_handler(&p, cs_value);
  my_xml_set_leave_handler(&p, cs_leave);
  info.loader = loader;
  my_xml_set_user_data(&p, (void *)&info);
  rc = (my_xml_parse(&p, buf, len) == MY_XML_OK) ? false : true;
  my_xml_parser_free(&p);
  my_charset_file_free(&info);
  if (rc != MY_XML_OK) {
    const char *errstr = my_xml_error_string(&p);
    if (sizeof(loader->errarg) > 32 + strlen(errstr)) {
      sprintf(loader->errarg, "at line %d pos %d: %s",
              my_xml_error_lineno(&p) + 1, (int)my_xml_error_pos(&p),
              my_xml_error_string(&p));
    }
  }


// Source: ctype.cc
// Lines 738-761
