ulong *STDCALL mysql_fetch_lengths(MYSQL_RES *res) {
  MYSQL_ROW column;

  if (!(column = res->current_row)) return nullptr; /* Something is wrong */
  if (res->data)
    (*res->methods->fetch_lengths)(res->lengths, column, res->field_count);
  return res->lengths;
}


// Source: client.cc
// Lines 7839-7846
