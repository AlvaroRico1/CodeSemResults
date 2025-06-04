timestamp_t approxidate_relative(const char *date)
{
	struct timeval tv;
	timestamp_t timestamp;
	int offset;
	int errors = 0;

	if (!parse_date_basic(date, &timestamp, &offset))
		return timestamp;

	get_time(&tv);
	return approxidate_str(date, (const struct timeval *) &tv, &errors);
}


// Source: date.c
// Lines 1326-1338
