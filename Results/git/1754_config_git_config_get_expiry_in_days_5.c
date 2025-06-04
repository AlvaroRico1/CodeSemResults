int git_config_get_expiry_in_days(const char *key, timestamp_t *expiry, timestamp_t now)
{
	const char *expiry_string;
	intmax_t days;
	timestamp_t when;

	if (git_config_get_string_tmp(key, &expiry_string))
		return 1; /* no such thing */

	if (git_parse_signed(expiry_string, &days, maximum_signed_value_of_type(int))) {
		const int scale = 86400;
		*expiry = now - days * scale;
		return 0;
	}

	if (!parse_expiry_date(expiry_string, &when)) {
		*expiry = when;
		return 0;
	}


// Source: config.c
// Lines 2458-2476
