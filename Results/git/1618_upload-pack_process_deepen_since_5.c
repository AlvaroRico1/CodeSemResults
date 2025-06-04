static int process_deepen_since(const char *line, timestamp_t *deepen_since, int *deepen_rev_list)
{
	const char *arg;
	if (skip_prefix(line, "deepen-since ", &arg)) {
		char *end = NULL;
		*deepen_since = parse_timestamp(arg, &end, 0);
		if (!end || *end || !deepen_since ||
		    /* revisions.c's max_age -1 is special */
		    *deepen_since == -1)
			die("Invalid deepen-since: %s", line);
		*deepen_rev_list = 1;
		return 1;
	}


// Source: upload-pack.c
// Lines 956-968
