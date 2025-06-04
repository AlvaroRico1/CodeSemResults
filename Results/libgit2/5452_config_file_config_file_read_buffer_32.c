static int config_file_read_buffer(
	git_config_entries *entries,
	const git_repository *repo,
	config_file *file,
	git_config_level_t level,
	int depth,
	const char *buf,
	size_t buflen)
{
	config_file_parse_data parse_data;
	git_config_parser reader;
	int error;

	if (depth >= MAX_INCLUDE_DEPTH) {
		git_error_set(GIT_ERROR_CONFIG, "maximum config include depth reached");
		return -1;
	}

	/* Initialize the reading position */
	reader.path = file->path;
	git_parse_ctx_init(&reader.ctx, buf, buflen);

	/* If the file is empty, there's nothing for us to do */
	if (!reader.ctx.content || *reader.ctx.content == '\0') {
		error = 0;
		goto out;
	}

	parse_data.repo = repo;
	parse_data.file = file;
	parse_data.entries = entries;
	parse_data.level = level;
	parse_data.depth = depth;

	error = git_config_parse(&reader, NULL, read_on_variable, NULL, NULL, &parse_data);

out:
	return error;
}


// Source: config_file.c
// Lines 824-862
