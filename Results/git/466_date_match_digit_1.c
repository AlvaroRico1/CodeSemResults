static int match_digit(const char *date, struct tm *tm, int *offset, int *tm_gmt)
{
	int n;
	char *end;
	timestamp_t num;

	num = parse_timestamp(date, &end, 10);

	/*
	 * Seconds since 1970? We trigger on that for any numbers with
	 * more than 8 digits. This is because we don't want to rule out
	 * numbers like 20070606 as a YYYYMMDD date.
	 */
	if (num >= 100000000 && nodate(tm)) {
		time_t time = num;
		if (gmtime_r(&time, tm)) {
			*tm_gmt = 1;
			return end - date;
		}
	}


// Source: date.c
// Lines 644-663
