int git_config_snapshot(git_config **out, git_config *in)
{
	int error = 0;
	size_t i;
	backend_internal *internal;
	git_config *config;

	*out = NULL;

	if (git_config_new(&config) < 0)
		return -1;

	git_vector_foreach(&in->backends, i, internal) {
		git_config_backend *b;

		if ((error = internal->backend->snapshot(&b, internal->backend)) < 0)
			break;

		if ((error = git_config_add_backend(config, b, internal->level, NULL, 0)) < 0) {
			b->free(b);
			break;
		}
	}


// Source: config.c
// Lines 153-175
