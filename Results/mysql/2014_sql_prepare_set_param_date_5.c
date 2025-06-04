static void set_param_date(Item_param *param, uchar **pos, ulong len) {
  MYSQL_TIME tm;

  if (len >= 4) {
    uchar *to = *pos;

    tm.year = (uint)sint2korr(to);
    tm.month = (uint)to[2];
    tm.day = (uint)to[3];

    tm.hour = tm.minute = tm.second = 0;
    tm.second_part = 0;
    tm.neg = false;
  } else


// Source: sql_prepare.cc
// Lines 607-620
