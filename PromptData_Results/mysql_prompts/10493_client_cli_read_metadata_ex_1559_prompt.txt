[ROLE]
Pointer analyzer that analyzes whether two pointer in a program alias

[QUESTION]
Below is a C program. Please analyze the alias relationship between:
1. Pointer 'pos' at line 5
2. Pointer 'fields' at line 6

Determine if these pointers MUST alias (always point to the same location), MAY alias (might point to the same location), or MUST-NOT alias (never point to the same location).

Program to analyze:
MYSQL_FIELD *cli_read_metadata_ex(MYSQL *mysql, MEM_ROOT *alloc,
                                  ulong field_count, unsigned int field) {
  ulong *len;
  uint f;
  uchar *pos;
  MYSQL_FIELD *fields, *result;
  MYSQL_ROWS data;
  NET *net = &mysql->net;
  size_t size;

  DBUG_TRACE;

  len = (ulong *)alloc->Alloc(sizeof(ulong) * field);
  size = sizeof(MYSQL_FIELD) * field_count;

  if (field_count != (size / sizeof(MYSQL_FIELD))) {
    set_mysql_error(mysql, CR_MALFORMED_PACKET, unknown_sqlstate);
    end_server(mysql);
    return nullptr;
  }

  fields = result = (MYSQL_FIELD *)alloc->Alloc(size);
  if (!result) {
    set_mysql_error(mysql, CR_OUT_OF_MEMORY, unknown_sqlstate);
    end_server(mysql);
    return nullptr;
  }
  memset(fields, 0, sizeof(MYSQL_FIELD) * field_count);

  data.data = (MYSQL_ROW)alloc->Alloc(sizeof(char *) * (field + 1));
  memset(data.data, 0, sizeof(char *) * (field + 1));

  /*
    In this below loop we read each column info as 1 single row
    and save it in mysql->fields array
  */
  for (f = 0; f < field_count; ++f) {
    if (read_one_row(mysql, field, data.data, len) == -1) return nullptr;
    if (unpack_field(mysql, alloc, false, mysql->server_capabilities, &data,
                     fields++))
      return nullptr;
  }
  /* Read EOF packet in case of old client */
  if (!(mysql->server_capabilities & CLIENT_DEPRECATE_EOF)) {
    if (packet_error == cli_safe_read(mysql, nullptr)) return nullptr;
    pos = net->read_pos;
    if (*pos == 254) {
      mysql->warning_count = uint2korr(pos + 1);
      mysql->server_status = uint2korr(pos + 3);
    }
  }
  return result;
}


// Source: client.cc
// Lines 2569-2621


[ANSWER]
MUST-NOT
