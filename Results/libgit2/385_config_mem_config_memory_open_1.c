static int config_memory_open(git_config_backend *backend, git_config_level_t level, const git_repository *repo)
{
	config_memory_backend *memory_backend = (config_memory_backend *) backend;
	git_config_parser parser = GIT_PARSE_CTX_INIT;
	config_memory_parse_data parse_data;
	int error;

	GIT_UNUSED(repo);

	if ((error = git_config_parser_init(&parser, "in-memory", memory_backend->cfg.ptr,
					    memory_backend->cfg.size)) < 0)
		goto out;
	parse_data.entries = memory_backend->entries;
	parse_data.level = level;

	if ((error = git_config_parse(&parser, NULL, read_variable_cb, NULL, NULL, &parse_data)) < 0)
		goto out;

out:
	git_config_parser_dispose(&parser);
	return error;
}


// Source: config_mem.c
// Lines 78-99
