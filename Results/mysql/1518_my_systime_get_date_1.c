void get_date(char *to, int flag, time_t date) {
  struct tm *start_time;
  time_t skr;
  struct tm tm_tmp;

  skr = date ? (time_t)date : my_time(0);
  if (flag & GETDATE_GMT)
    gmtime_r(&skr, &tm_tmp);
  else
    localtime_r(&skr, &tm_tmp);
  start_time = &tm_tmp;

  if (flag & GETDATE_SHORT_DATE) {
    to += std::sprintf(to, "%02d%02d%02d", start_time->tm_year % 100,
                       start_time->tm_mon + 1, start_time->tm_mday);
  } else if (flag & GETDATE_SHORT_DATE_FULL_YEAR) {
    to += std::sprintf(to, "%4d%02d%02d", start_time->tm_year + 1900,
                       start_time->tm_mon + 1, start_time->tm_mday);
  } else {
    to += std::sprintf(
        to, ((flag & GETDATE_FIXEDLENGTH) ? "%4d-%02d-%02d" : "%d-%02d-%02d"),
        start_time->tm_year + 1900, start_time->tm_mon + 1,
        start_time->tm_mday);
  }
  if (flag & GETDATE_DATE_TIME) {
    if (flag & GETDATE_T_DELIMITER) {
      *to = 'T';
      ++to;
    }
    to += std::sprintf(
        to,
        ((flag & GETDATE_FIXEDLENGTH) ? " %02d:%02d:%02d" : " %2d:%02d:%02d"),
        start_time->tm_hour, start_time->tm_min, start_time->tm_sec);
  } else if (flag & GETDATE_HHMMSSTIME) {
    if (flag & GETDATE_T_DELIMITER) {
      *to = 'T';
      ++to;
    }
    to += std::sprintf(to, "%02d%02d%02d", start_time->tm_hour,
                       start_time->tm_min, start_time->tm_sec);
  }
}


// Source: my_systime.cc
// Lines 107-148
