static bool is_ipv6(const char *str)
{
	const char *c;
	size_t colons = 0;

	if (*str++ != '[')
		return false;

	for (c = str; *c; c++) {
		if (*c  == ':')
			colons++;

		if (*c == ']')
			return (colons > 1);

		if (*c != ':' &&
		    (*c < '0' || *c > '9') &&
		    (*c < 'a' || *c > 'f') &&
		    (*c < 'A' || *c > 'F'))
			return false;
	}

	return false;
}


// Source: net.c
// Lines 220-243
