args_string_percentage(const char *value, long long minval, long long maxval,
    long long curval, char **cause)
{
	const char	*errstr;
	long long	 ll;
	size_t		 valuelen = strlen(value);
	char		*copy;

	if (value[valuelen - 1] == '%') {
		copy = xstrdup(value);
		copy[valuelen - 1] = '\0';

		ll = strtonum(copy, 0, 100, &errstr);
		free(copy);
		if (errstr != NULL) {
			*cause = xstrdup(errstr);
			return (0);
		}
		ll = (curval * ll) / 100;
		if (ll < minval) {
			*cause = xstrdup("too small");
			return (0);
		}
		if (ll > maxval) {
			*cause = xstrdup("too large");
			return (0);
		}
	} else {
		ll = strtonum(value, minval, maxval, &errstr);
		if (errstr != NULL) {
			*cause = xstrdup(errstr);
			return (0);
		}
	}

	*cause = NULL;
	return (ll);
}


// Source: arguments.c
// Lines 851-888
