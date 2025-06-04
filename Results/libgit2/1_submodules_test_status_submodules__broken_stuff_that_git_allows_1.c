void test_status_submodules__broken_stuff_that_git_allows(void)
{
	git_status_options opts = GIT_STATUS_OPTIONS_INIT;
	status_entry_counts counts;
	git_repository *contained;
	static const char *expected_files_with_broken[] = {
		".gitmodules",
		"added",
		"broken/tracked",
		"deleted",
		"ignored",
		"modified",
		"untracked"
	};
	static unsigned int expected_status_with_broken[] = {
		GIT_STATUS_WT_MODIFIED,
		GIT_STATUS_INDEX_NEW,
		GIT_STATUS_INDEX_NEW,
		GIT_STATUS_INDEX_DELETED,
		GIT_STATUS_IGNORED,
		GIT_STATUS_WT_MODIFIED,
		GIT_STATUS_WT_NEW,
	};

	g_repo = setup_fixture_submodules();

	opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
		GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS |
		GIT_STATUS_OPT_INCLUDE_IGNORED;

	/* make a directory and stick a tracked item into the index */
	{
		git_index *idx;
		cl_must_pass(p_mkdir("submodules/broken", 0777));
		cl_git_mkfile("submodules/broken/tracked", "tracked content");
		cl_git_pass(git_repository_index(&idx, g_repo));
		cl_git_pass(git_index_add_bypath(idx, "broken/tracked"));
		cl_git_pass(git_index_write(idx));
		git_index_free(idx);
	}


// Source: submodules.c
// Lines 391-430
