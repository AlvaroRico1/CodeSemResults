timestamp_t approxidate_careful(const char *date, int *error_ret)
{
	struct timeval tv;
	timestamp_t timestamp;
	int offset;
	int dummy = 0;
	if (!error_ret)
		error_ret = &dummy;

	if (!parse_date_basic(date, &timestamp, &offset)) {
		*error_ret = 0;
		return timestamp;
	}

	get_time(&tv);
	return approxidate_str(date, &tv, error_ret);
}


// Source: date.c
// Lines 1340-1356
