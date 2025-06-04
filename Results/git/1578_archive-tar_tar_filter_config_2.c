static int tar_filter_config(const char *var, const char *value, void *data)
{
	struct archiver *ar;
	const char *name;
	const char *type;
	size_t namelen;

	if (parse_config_key(var, "tar", &name, &namelen, &type) < 0 || !name)
		return 0;

	ar = find_tar_filter(name, namelen);
	if (!ar) {
		CALLOC_ARRAY(ar, 1);
		ar->name = xmemdupz(name, namelen);
		ar->write_archive = write_tar_filter_archive;
		ar->flags = ARCHIVER_WANT_COMPRESSION_LEVELS |
			    ARCHIVER_HIGH_COMPRESSION_LEVELS;
		ALLOC_GROW(tar_filters, nr_tar_filters + 1, alloc_tar_filters);
		tar_filters[nr_tar_filters++] = ar;
	}

	if (!strcmp(type, "command")) {
		if (!value)
			return config_error_nonbool(var);
		free(ar->data);
		ar->data = xstrdup(value);
		return 0;
	}
	if (!strcmp(type, "remote")) {
		if (git_config_bool(var, value))
			ar->flags |= ARCHIVER_REMOTE;
		else
			ar->flags &= ~ARCHIVER_REMOTE;
		return 0;
	}

	return 0;
}


// Source: archive-tar.c
// Lines 362-399
