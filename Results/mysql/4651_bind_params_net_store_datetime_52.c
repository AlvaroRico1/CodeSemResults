static void net_store_datetime(NET *net, MYSQL_TIME *tm) {
  uchar buff[MAX_DATETIME_REP_LENGTH], *pos;
  // The content of the buffer's length byte.
  uchar length_byte;

  pos = buff + 1;

  int2store(pos, static_cast<std::uint16_t>(tm->year));
  pos[2] = static_cast<std::uint8_t>(tm->month);
  pos[3] = static_cast<std::uint8_t>(tm->day);
  pos[4] = static_cast<std::uint8_t>(tm->hour);
  pos[5] = static_cast<std::uint8_t>(tm->minute);
  pos[6] = static_cast<std::uint8_t>(tm->second);
  int4store(pos + 7, static_cast<std::uint32_t>(tm->second_part));
  if (tm->time_type == MYSQL_TIMESTAMP_DATETIME_TZ) {
    int tzd = tm->time_zone_displacement;
    assert(tzd % SECS_PER_MIN == 0);
    assert(std::abs(tzd) <= MAX_TIME_ZONE_HOURS * SECS_PER_HOUR);
    int2store(pos + 11, static_cast<std::uint16_t>(tzd / SECS_PER_MIN));
    length_byte = 13;
  } else if (tm->second_part)


// Source: bind_params.cc
// Lines 176-196
