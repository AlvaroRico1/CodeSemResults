int git_config_parse_int64(int64_t *out, const char *value)
{
	const char *num_end;
	int64_t num;

	if (!value || git__strntol64(&num, value, strlen(value), &num_end, 0) < 0)
		goto fail_parse;

	switch (*num_end) {
	case 'g':
	case 'G':
		num *= 1024;
		/* fallthrough */

	case 'm':
	case 'M':
		num *= 1024;
		/* fallthrough */

	case 'k':
	case 'K':
		num *= 1024;

		/* check that that there are no more characters after the
		 * given modifier suffix */
		if (num_end[1] != '\0')
			return -1;

		/* fallthrough */

	case '\0':
		*out = num;
		return 0;

	default:
		goto fail_parse;
	}

fail_parse:
	git_error_set(GIT_ERROR_CONFIG, "failed to parse '%s' as an integer", value ? value : "(null)");
	return -1;
}


// Source: config.c
// Lines 1366-1407
