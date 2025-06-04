	struct string_list offsets;

	/*
	 * last_directory: directory that previously processed file found in
	 *
	 * last_directory starts NULL, but records the directory in which the
	 * previous file was found within.  As soon as
	 *    directory(current_file) != last_directory
	 * then we need to start updating accounting in versions & offsets.
	 * Note that last_directory is always the last path in "offsets" (or
	 * NULL if "offsets" is empty) so this exists just for quick access.
	 */
	const char *last_directory;

	/* last_directory_len: cached computation of strlen(last_directory) */
	unsigned last_directory_len;
};


// Source: merge-ort.c
// Lines 3293-3309
