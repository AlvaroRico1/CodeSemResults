static int is_scissors_line(const char *line)
{
	const char *c;
	int scissors = 0, gap = 0;
	const char *first_nonblank = NULL, *last_nonblank = NULL;
	int visible, perforation = 0, in_perforation = 0;

	for (c = line; *c; c++) {
		if (isspace(*c)) {
			if (in_perforation) {
				perforation++;
				gap++;
			}
			continue;
		}
		last_nonblank = c;
		if (first_nonblank == NULL)
			first_nonblank = c;
		if (*c == '-') {
			in_perforation = 1;
			perforation++;
			continue;
		}
		if (starts_with(c, ">8") || starts_with(c, "8<") ||
		    starts_with(c, ">%") || starts_with(c, "%<")) {
			in_perforation = 1;
			perforation += 2;
			scissors += 2;
			c++;
			continue;
		}
		in_perforation = 0;
	}

	/*
	 * The mark must be at least 8 bytes long (e.g. "-- >8 --").
	 * Even though there can be arbitrary cruft on the same line
	 * (e.g. "cut here"), in order to avoid misidentification, the
	 * perforation must occupy more than a third of the visible
	 * width of the line, and dashes and scissors must occupy more
	 * than half of the perforation.
	 */

	if (first_nonblank && last_nonblank)
		visible = last_nonblank - first_nonblank + 1;
	else
		visible = 0;
	return (scissors && 8 <= visible &&
		visible < perforation * 3 &&
		gap * 2 < perforation);
}


// Source: mailinfo.c
// Lines 685-735
