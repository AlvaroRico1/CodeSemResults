int git_filter_list_new(
	git_filter_list **out,
	git_repository *repo,
	git_filter_mode_t mode,
	uint32_t flags)
{
	git_filter_source src = { 0 };
	src.repo = repo;
	src.path = NULL;
	src.mode = mode;
	src.options.flags = flags;
	return filter_list_new(out, &src);
}


// Source: filter.c
// Lines 496-508
