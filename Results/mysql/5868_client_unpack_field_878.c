static int unpack_field(MYSQL *mysql, MEM_ROOT *alloc, bool default_value,
                        uint server_capabilities, MYSQL_ROWS *row,
                        MYSQL_FIELD *field) {
  ulong lengths[9]; /* Max length of each field */
  DBUG_TRACE;

  if (!field) {
    set_mysql_error(mysql, CR_UNKNOWN_ERROR, unknown_sqlstate);
    return 1;
  }

  memset(field, 0, sizeof(MYSQL_FIELD));

  if (server_capabilities & CLIENT_PROTOCOL_41) {
    uchar *pos;
    /* fields count may be wrong */
    cli_fetch_lengths(&lengths[0], row->data, default_value ? 8 : 7);
    field->catalog = strmake_root(alloc, (char *)row->data[0], lengths[0]);
    field->db = strmake_root(alloc, (char *)row->data[1], lengths[1]);
    field->table = strmake_root(alloc, (char *)row->data[2], lengths[2]);
    field->org_table = strmake_root(alloc, (char *)row->data[3], lengths[3]);
    field->name = strmake_root(alloc, (char *)row->data[4], lengths[4]);
    field->org_name = strmake_root(alloc, (char *)row->data[5], lengths[5]);

    field->catalog_length = lengths[0];
    field->db_length = lengths[1];
    field->table_length = lengths[2];
    field->org_table_length = lengths[3];
    field->name_length = lengths[4];
    field->org_name_length = lengths[5];

    /* Unpack fixed length parts */
    if (lengths[6] != 12) {
      /* malformed packet. signal an error. */
      set_mysql_error(mysql, CR_MALFORMED_PACKET, unknown_sqlstate);
      return 1;
    }

    pos = (uchar *)row->data[6];
    field->charsetnr = uint2korr(pos);
    field->length = (uint)uint4korr(pos + 2);
    field->type = (enum enum_field_types)pos[6];
    field->flags = uint2korr(pos + 7);
    field->decimals = (uint)pos[9];

    if (IS_NUM(field->type)) field->flags |= NUM_FLAG;
    if (default_value && row->data[7]) {
      field->def = strmake_root(alloc, (char *)row->data[7], lengths[7]);
      field->def_length = lengths[7];
    } else
      field->def = nullptr;
    field->max_length = 0;
  }


// Source: client.cc
// Lines 2317-2369
