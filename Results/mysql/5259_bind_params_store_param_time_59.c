static void store_param_time(NET *net, MYSQL_BIND *param) {
  MYSQL_TIME *tm = (MYSQL_TIME *)param->buffer;
  uchar buff[MAX_TIME_REP_LENGTH], *pos;
  uint length;

  pos = buff + 1;
  pos[0] = tm->neg ? 1 : 0;
  int4store(pos + 1, tm->day);
  pos[5] = (uchar)tm->hour;
  pos[6] = (uchar)tm->minute;
  pos[7] = (uchar)tm->second;
  int4store(pos + 8, tm->second_part);
  if (tm->second_part)
    length = 12;
  else if (tm->hour || tm->minute || tm->second || tm->day)
    length = 8;
  else
    length = 0;
  buff[0] = (char)length++;
  memcpy((char *)net->write_pos, buff, length);
  net->write_pos += length;
}


// Source: bind_params.cc
// Lines 153-174
