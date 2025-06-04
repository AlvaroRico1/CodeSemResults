int git_tree__parse_raw(void *_tree, const char *data, size_t size)
{
	git_tree *tree = _tree;
	const char *buffer;
	const char *buffer_end;

	buffer = data;
	buffer_end = buffer + size;

	tree->odb_obj = NULL;
	git_array_init_to_size(tree->entries, DEFAULT_TREE_SIZE);
	GIT_ERROR_CHECK_ARRAY(tree->entries);

	while (buffer < buffer_end) {
		git_tree_entry *entry;
		size_t filename_len;
		const char *nul;
		uint16_t attr;

		if (parse_mode(&attr, buffer, buffer_end - buffer, &buffer) < 0 || !buffer)
			return tree_parse_error("failed to parse tree: can't parse filemode", NULL);

		if (buffer >= buffer_end || (*buffer++) != ' ')
			return tree_parse_error("failed to parse tree: missing space after filemode", NULL);

		if ((nul = memchr(buffer, 0, buffer_end - buffer)) == NULL)
			return tree_parse_error("failed to parse tree: object is corrupted", NULL);

		if ((filename_len = nul - buffer) == 0 || filename_len > UINT16_MAX)
			return tree_parse_error("failed to parse tree: can't parse filename", NULL);

		if ((buffer_end - (nul + 1)) < GIT_OID_RAWSZ)
			return tree_parse_error("failed to parse tree: can't parse OID", NULL);

		/* Allocate the entry */
		{
			entry = git_array_alloc(tree->entries);
			GIT_ERROR_CHECK_ALLOC(entry);

			entry->attr = attr;
			entry->filename_len = (uint16_t)filename_len;
			entry->filename = buffer;
			entry->oid = (git_oid *) ((char *) buffer + filename_len + 1);
		}

		buffer += filename_len + 1;
		buffer += GIT_OID_RAWSZ;
	}


// Source: tree.c
// Lines 393-440
