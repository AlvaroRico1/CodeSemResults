static timestamp_t approxidate_str(const char *date,
				   const struct timeval *tv,
				   int *error_ret)
{
	int number = 0;
	int touched = 0;
	struct tm tm, now;
	time_t time_sec;

	time_sec = tv->tv_sec;
	localtime_r(&time_sec, &tm);
	now = tm;

	tm.tm_year = -1;
	tm.tm_mon = -1;
	tm.tm_mday = -1;

	for (;;) {
		unsigned char c = *date;
		if (!c)
			break;
		date++;
		if (isdigit(c)) {
			pending_number(&tm, &number);
			date = approxidate_digit(date-1, &tm, &number, time_sec);
			touched = 1;
			continue;
		}
		if (isalpha(c))
			date = approxidate_alpha(date-1, &tm, &now, &number, &touched);
	}
	pending_number(&tm, &number);
	if (!touched)
		*error_ret = 1;
	return (timestamp_t)update_tm(&tm, &now, 0);
}


// Source: date.c
// Lines 1289-1324
