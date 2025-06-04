bool git_net_str_is_url(const char *str)
{
	const char *c;

	for (c = str; *c; c++) {
		if (*c == ':' && *(c+1) == '/' && *(c+2) == '/')
			return true;

		if ((*c < 'a' || *c > 'z') &&
		    (*c < 'A' || *c > 'Z') &&
		    (*c < '0' || *c > '9') &&
		    (*c != '+' && *c != '-' && *c != '.'))
			break;
	}

	return false;
}


// Source: net.c
// Lines 22-38
