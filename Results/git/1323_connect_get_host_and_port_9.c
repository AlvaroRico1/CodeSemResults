static void get_host_and_port(char **host, const char **port)
{
	char *colon, *end;
	end = host_end(host, 1);
	colon = strchr(end, ':');
	if (colon) {
		long portnr = strtol(colon + 1, &end, 10);
		if (end != colon + 1 && *end == '\0' && 0 <= portnr && portnr < 65536) {
			*colon = 0;
			*port = colon + 1;
		} else if (!colon[1]) {
			*colon = 0;
		}
	}
}


// Source: connect.c
// Lines 700-714
