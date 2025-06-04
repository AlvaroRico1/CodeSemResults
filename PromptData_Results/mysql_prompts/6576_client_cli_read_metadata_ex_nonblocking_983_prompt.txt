[ROLE]
Pointer analyzer that analyzes whether two pointer in a program alias

[QUESTION]
Below is a C program. Please analyze the alias relationship between:
1. Pointer 'res' at line 41
2. Pointer 'mysql' at line 1

Determine if these pointers MUST alias (always point to the same location), MAY alias (might point to the same location), or MUST-NOT alias (never point to the same location).

Program to analyze:
net_async_status cli_read_metadata_ex_nonblocking(MYSQL *mysql, MEM_ROOT *alloc,
                                                  ulong field_count,
                                                  unsigned int field,
                                                  MYSQL_FIELD **ret) {
  DBUG_TRACE;
  uchar *pos;
  ulong pkt_len;
  NET *net = &mysql->net;
  MYSQL_ASYNC *async_data = ASYNC_DATA(mysql);
  *ret = nullptr;

  if (!async_data->async_read_metadata_field_len) {
    async_data->async_read_metadata_field_len =
        (ulong *)alloc->Alloc(sizeof(ulong) * field);
  }
  if (!async_data->async_read_metadata_fields) {
    async_data->async_read_metadata_fields =
        (MYSQL_FIELD *)alloc->Alloc((uint)sizeof(MYSQL_FIELD) * field_count);
    if (async_data->async_read_metadata_fields)
      memset(async_data->async_read_metadata_fields, 0,
             sizeof(MYSQL_FIELD) * field_count);
  }

  if (!async_data->async_read_metadata_fields) {
    set_mysql_error(mysql, CR_OUT_OF_MEMORY, unknown_sqlstate);
    goto end;
  }

  if (!async_data->async_read_metadata_data.data) {
    async_data->async_read_metadata_data.data =
        (MYSQL_ROW)alloc->Alloc(sizeof(char *) * (field + 1));
    memset(async_data->async_read_metadata_data.data, 0,
           sizeof(char *) * (field + 1));
  }

  /*
    In this below loop we read each column info as 1 single row
    and save it in mysql->fields array
  */
  while (async_data->async_read_metadata_cur_field < field_count) {
    int res;
    if (read_one_row_nonblocking(mysql, field,
                                 async_data->async_read_metadata_data.data,
                                 async_data->async_read_metadata_field_len,
                                 &res) == NET_ASYNC_NOT_READY) {
      return NET_ASYNC_NOT_READY;
    }

    if (res == -1) {
      goto end;
    }

    if (unpack_field(mysql, alloc, false, mysql->server_capabilities,
                     &async_data->async_read_metadata_data,
                     async_data->async_read_metadata_fields +
                         async_data->async_read_metadata_cur_field)) {
      goto end;
    }
    async_data->async_read_metadata_cur_field++;
  }


// Source: client.cc
// Lines 2468-2527


[ANSWER]
MUST-NOT
