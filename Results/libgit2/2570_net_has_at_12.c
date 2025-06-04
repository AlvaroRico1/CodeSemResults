static bool has_at(const char *str)
{
	const char *c;

	for (c = str; *c; c++) {
		if (*c == '@')
			return true;

		if (*c == ':')
			break;
	}

	return false;
}


// Source: net.c
// Lines 245-258
