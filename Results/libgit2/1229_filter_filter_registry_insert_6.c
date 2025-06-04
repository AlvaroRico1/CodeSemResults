static int filter_registry_insert(
	const char *name, git_filter *filter, int priority)
{
	git_filter_def *fdef;
	size_t nattr = 0, nmatch = 0, alloc_len;
	git_str attrs = GIT_STR_INIT;

	if (filter_def_scan_attrs(&attrs, &nattr, &nmatch, filter->attributes) < 0)
		return -1;

	GIT_ERROR_CHECK_ALLOC_MULTIPLY(&alloc_len, nattr, 2);
	GIT_ERROR_CHECK_ALLOC_MULTIPLY(&alloc_len, alloc_len, sizeof(char *));
	GIT_ERROR_CHECK_ALLOC_ADD(&alloc_len, alloc_len, sizeof(git_filter_def));

	fdef = git__calloc(1, alloc_len);
	GIT_ERROR_CHECK_ALLOC(fdef);

	fdef->filter_name = git__strdup(name);
	GIT_ERROR_CHECK_ALLOC(fdef->filter_name);

	fdef->filter      = filter;
	fdef->priority    = priority;
	fdef->nattrs      = nattr;
	fdef->nmatches    = nmatch;
	fdef->attrdata    = git_str_detach(&attrs);

	filter_def_set_attrs(fdef);

	if (git_vector_insert(&filter_registry.filters, fdef) < 0) {
		git__free(fdef->filter_name);
		git__free(fdef->attrdata);
		git__free(fdef);
		return -1;
	}

	git_vector_sort(&filter_registry.filters);
	return 0;
}


// Source: filter.c
// Lines 152-189
