static int process_deepen(const char *line, int *depth)
{
	const char *arg;
	if (skip_prefix(line, "deepen ", &arg)) {
		char *end = NULL;
		*depth = (int)strtol(arg, &end, 0);
		if (!end || *end || *depth <= 0)
			die("Invalid deepen: %s", line);
		return 1;
	}


// Source: upload-pack.c
// Lines 942-951
