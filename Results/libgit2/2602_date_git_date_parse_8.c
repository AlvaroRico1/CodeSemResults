int git_date_parse(git_time_t *out, const char *date)
{
	time_t time_sec;
	git_time_t timestamp;
	int offset, error_ret=0;

	if (!parse_date_basic(date, &timestamp, &offset)) {
		*out = timestamp;
		return 0;
	}

	if (time(&time_sec) == -1)
		return -1;

	*out = approxidate_str(date, time_sec, &error_ret);
	return error_ret;
}


// Source: date.c
// Lines 861-877
