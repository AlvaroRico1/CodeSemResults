static void set_param_time(Item_param *param, uchar **pos, ulong len) {
  MYSQL_TIME tm;

  if (len >= 8) {
    uchar *to = *pos;
    uint day;

    tm.neg = (bool)to[0];
    day = (uint)sint4korr(to + 1);
    tm.hour = (uint)to[5] + day * 24;
    tm.minute = (uint)to[6];
    tm.second = (uint)to[7];
    tm.second_part = (len > 8) ? (ulong)sint4korr(to + 8) : 0;
    if (tm.hour > 838) {
      /* TODO: add warning 'Data truncated' here */
      tm.hour = 838;
      tm.minute = 59;
      tm.second = 59;
    }
    tm.day = tm.year = tm.month = 0;
  } else


// Source: sql_prepare.cc
// Lines 551-571
