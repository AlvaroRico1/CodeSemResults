static int cs_leave(MY_XML_PARSER *st, const char *attr, size_t len) {
  struct my_cs_file_info *i = (struct my_cs_file_info *)st->user_data;
  struct my_cs_file_section_st *s = cs_file_sec(attr, len);
  int state = s ? s->state : 0;
  int rc;

  switch (state) {
    case _CS_COLLATION:
      if (i->tailoring_length) i->cs.tailoring = i->tailoring;
      rc = i->loader->add_collation ? i->loader->add_collation(&i->cs)
                                    : MY_XML_OK;
      break;

    /* Rules: Logical Reset Positions */
    case _CS_RESET_FIRST_NON_IGNORABLE:
      rc = tailoring_append(st, "[first non-ignorable]", 0, nullptr);
      break;

    case _CS_RESET_LAST_NON_IGNORABLE:
      rc = tailoring_append(st, "[last non-ignorable]", 0, nullptr);
      break;

    case _CS_RESET_FIRST_PRIMARY_IGNORABLE:
      rc = tailoring_append(st, "[first primary ignorable]", 0, nullptr);
      break;

    case _CS_RESET_LAST_PRIMARY_IGNORABLE:
      rc = tailoring_append(st, "[last primary ignorable]", 0, nullptr);
      break;

    case _CS_RESET_FIRST_SECONDARY_IGNORABLE:
      rc = tailoring_append(st, "[first secondary ignorable]", 0, nullptr);
      break;

    case _CS_RESET_LAST_SECONDARY_IGNORABLE:
      rc = tailoring_append(st, "[last secondary ignorable]", 0, nullptr);
      break;

    case _CS_RESET_FIRST_TERTIARY_IGNORABLE:
      rc = tailoring_append(st, "[first tertiary ignorable]", 0, nullptr);
      break;

    case _CS_RESET_LAST_TERTIARY_IGNORABLE:
      rc = tailoring_append(st, "[last tertiary ignorable]", 0, nullptr);
      break;

    case _CS_RESET_FIRST_TRAILING:
      rc = tailoring_append(st, "[first trailing]", 0, nullptr);
      break;

    case _CS_RESET_LAST_TRAILING:
      rc = tailoring_append(st, "[last trailing]", 0, nullptr);
      break;

    case _CS_RESET_FIRST_VARIABLE:
      rc = tailoring_append(st, "[first variable]", 0, nullptr);
      break;

    case _CS_RESET_LAST_VARIABLE:
      rc = tailoring_append(st, "[last variable]", 0, nullptr);
      break;

    default:
      rc = MY_XML_OK;
  }
  return rc;
}


// Source: ctype.cc
// Lines 456-522
