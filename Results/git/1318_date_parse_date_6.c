int parse_date(const char *date, struct strbuf *result)
{
	timestamp_t timestamp;
	int offset;
	if (parse_date_basic(date, &timestamp, &offset))
		return -1;
	date_string(timestamp, offset, result);
	return 0;
}


// Source: date.c
// Lines 923-931
