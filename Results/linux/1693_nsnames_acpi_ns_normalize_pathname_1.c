static void acpi_ns_normalize_pathname(char *original_path)
{
	char *input_path = original_path;
	char *new_path_buffer;
	char *new_path;
	u32 i;

	/* Allocate a temp buffer in which to construct the new path */

	new_path_buffer = ACPI_ALLOCATE_ZEROED(strlen(input_path) + 1);
	new_path = new_path_buffer;
	if (!new_path_buffer) {
		return;
	}

	/* Special characters may appear at the beginning of the path */

	if (*input_path == '\\') {
		*new_path = *input_path;
		new_path++;
		input_path++;
	}

	while (*input_path == '^') {
		*new_path = *input_path;
		new_path++;
		input_path++;
	}

	/* Remainder of the path */

	while (*input_path) {

		/* Do one nameseg at a time */

		for (i = 0; (i < ACPI_NAMESEG_SIZE) && *input_path; i++) {
			if ((i == 0) || (*input_path != '_')) {	/* First char is allowed to be underscore */
				*new_path = *input_path;
				new_path++;
			}

			input_path++;
		}

		/* Dot means that there are more namesegs to come */

		if (*input_path == '.') {
			*new_path = *input_path;
			new_path++;
			input_path++;
		}
	}

	*new_path = 0;
	strcpy(original_path, new_path_buffer);
	ACPI_FREE(new_path_buffer);
}


// Source: nsnames.c
// Lines 414-470
