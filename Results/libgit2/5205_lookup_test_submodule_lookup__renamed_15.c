void test_submodule_lookup__renamed(void)
{
	const char *newpath = "sm_actually_changed";
	git_index *idx;
	sm_lookup_data data;

	cl_git_pass(git_repository_index__weakptr(&idx, g_repo));

	/* We're replicating 'git mv sm_unchanged sm_actually_changed' in this test */

	cl_git_pass(p_rename("submod2/sm_unchanged", "submod2/sm_actually_changed"));

	/* Change the path in .gitmodules and stage it*/
	{
		git_config *cfg;

		cl_git_pass(git_config_open_ondisk(&cfg, "submod2/.gitmodules"));
		cl_git_pass(git_config_set_string(cfg, "submodule.sm_unchanged.path", newpath));
		git_config_free(cfg);

		cl_git_pass(git_index_add_bypath(idx, ".gitmodules"));
	}


// Source: lookup.c
// Lines 403-424
