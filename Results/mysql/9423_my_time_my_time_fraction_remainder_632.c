inline long my_time_fraction_remainder(long nr, unsigned int decimals) {
  assert(decimals <= DATETIME_MAX_DECIMALS);
  return nr % static_cast<long>(log_10_int[DATETIME_MAX_DECIMALS - decimals]);
}

/**
   Truncate the number of microseconds in MYSQL_TIME::second_part to
   the desired precision.

   @param ltime time point
   @param decimals desired precision
*/
inline void my_time_trunc(MYSQL_TIME *ltime, unsigned int decimals) {
  ltime->second_part -=
      my_time_fraction_remainder(ltime->second_part, decimals);
}

/**
   Alias for my_time_trunc.

   @param ltime time point
   @param decimals desired precision
 */
inline void my_datetime_trunc(MYSQL_TIME *ltime, unsigned int decimals) {
  return my_time_trunc(ltime, decimals);
}

/**
   Truncate the tv_usec member of a posix timeval struct to the
   specified number of decimals.

   @param tv timepoint/duration
   @param decimals desired precision
 */
inline void my_timeval_trunc(struct timeval *tv, unsigned int decimals) {
  tv->tv_usec -= my_time_fraction_remainder(tv->tv_usec, decimals);
}

/**
   Predicate for fuzzyness of date.

   @param my_time time point to check
   @param fuzzydate bitfield indicating if fuzzy dates are premitted
   @retval true if TIME_FUZZY_DATE is unset and either month or day is 0
   @retval false otherwise
 */
inline bool check_fuzzy_date(const MYSQL_TIME &my_time,
                             my_time_flags_t fuzzydate) {
  return !(fuzzydate & TIME_FUZZY_DATE) && (!my_time.month || !my_time.day);
}

/**
 Predicate which returns true if at least one of the date members are non-zero.

 @param my_time time point to check.
 @retval false if all the date members are zero
 @retval true otherwise
 */
inline bool non_zero_date(const MYSQL_TIME &my_time) {
  return my_time.year || my_time.month || my_time.day;
}

/**
 Predicate which returns true if at least one of the time members are non-zero.

 @param my_time time point to check.
 @retval false if all the time members are zero
 @retval true otherwise
*/
inline bool non_zero_time(const MYSQL_TIME &my_time) {
  return my_time.hour || my_time.minute || my_time.second ||
         my_time.second_part;
}

/**
   "Casts" MYSQL_TIME datetime to a MYSQL_TIME time. Sets
   MYSQL_TIME::time_type to MYSQL_TIMESTAMP_TIME and zeroes out the
   date members.

   @param ltime timepoint to cast
 */
inline void datetime_to_time(MYSQL_TIME *ltime) {
  ltime->year = 0;
  ltime->month = 0;
  ltime->day = 0;
  ltime->time_type = MYSQL_TIMESTAMP_TIME;
}

/**
   "Casts" MYSQL_TIME datetime to a MYSQL_TIME date. Sets
   MYSQL_TIME::time_type to MYSQL_TIMESTAMP_DATE and zeroes out the
   time members.

   @param ltime timepoint to cast
*/
inline void datetime_to_date(MYSQL_TIME *ltime) {
  ltime->hour = 0;
  ltime->minute = 0;
  ltime->second = 0;
  ltime->second_part = 0;
  ltime->time_type = MYSQL_TIMESTAMP_DATE;
}

/**
   "Casts" a MYSQL_TIME to datetime by setting MYSQL_TIME::time_type to
   MYSQL_TIMESTAMP_DATETIME.
   @note There is no check to ensure that the result is a valid datetime.

   @param ltime timpoint to cast
*/
inline void date_to_datetime(MYSQL_TIME *ltime) {
  ltime->time_type = MYSQL_TIMESTAMP_DATETIME;
}

bool time_add_nanoseconds_with_truncate(MYSQL_TIME *ltime,
                                        unsigned int nanoseconds,
                                        int *warnings);
bool datetime_add_nanoseconds_with_truncate(MYSQL_TIME *ltime,
                                            unsigned int nanoseconds);
bool time_add_nanoseconds_with_round(MYSQL_TIME *ltime,
                                     unsigned int nanoseconds, int *warnings);

bool datetime_add_nanoseconds_with_round(MYSQL_TIME *ltime,
                                         unsigned int nanoseconds,
                                         int *warnings);

bool time_add_nanoseconds_adjust_frac(MYSQL_TIME *ltime,
                                      unsigned int nanoseconds, int *warnings,
                                      bool truncate);

bool datetime_add_nanoseconds_adjust_frac(MYSQL_TIME *ltime,
                                          unsigned int nanoseconds,
                                          int *warnings, bool truncate);

bool my_time_adjust_frac(MYSQL_TIME *ltime, unsigned int dec, bool truncate);
bool my_datetime_adjust_frac(MYSQL_TIME *ltime, unsigned int dec, int *warnings,
                             bool truncate);
bool my_timeval_round(struct timeval *tv, unsigned int decimals);
void mix_date_and_time(MYSQL_TIME *ldate, const MYSQL_TIME &my_time);

void localtime_to_TIME(MYSQL_TIME *to, const struct tm *from);
void calc_time_from_sec(MYSQL_TIME *to, long long int seconds,
                        long microseconds);
bool calc_time_diff(const MYSQL_TIME &my_time1, const MYSQL_TIME &my_time2,
                    int l_sign, long long int *seconds_out,
                    long *microseconds_out);
int my_time_compare(const MYSQL_TIME &my_time_a, const MYSQL_TIME &my_time_b);

long long int TIME_to_longlong_packed(const MYSQL_TIME &my_time,
                                      enum enum_field_types type);

void TIME_from_longlong_packed(MYSQL_TIME *ltime, enum enum_field_types type,
                               long long int packed_value);

long long int longlong_from_datetime_packed(enum enum_field_types type,
                                            long long int packed_value);

double double_from_datetime_packed(enum enum_field_types type,
                                   long long int packed_value);

/**
  @} (end of ingroup MY_TIME)


// Source: my_time.h
// Lines 439-600
