void test_status_worktree_init__bracket_in_filename(void)
{
	git_repository *repo;
	git_index *index;
	status_entry_single result;
	unsigned int status_flags;

	#define FILE_WITH_BRACKET "LICENSE[1].md"
	#define FILE_WITHOUT_BRACKET "LICENSE1.md"

	cl_set_cleanup(&cleanup_new_repo, "with_bracket");

	cl_git_pass(git_repository_init(&repo, "with_bracket", 0));
	cl_git_mkfile("with_bracket/" FILE_WITH_BRACKET, "I have a bracket in my name\n");

	/* file is new to working directory */

	memset(&result, 0, sizeof(result));
	cl_git_pass(git_status_foreach(repo, cb_status__single, &result));
	cl_assert_equal_i(1, result.count);
	cl_assert(result.status == GIT_STATUS_WT_NEW);

	cl_git_pass(git_status_file(&status_flags, repo, FILE_WITH_BRACKET));
	cl_assert(status_flags == GIT_STATUS_WT_NEW);

	/* ignore the file */

	cl_git_rewritefile("with_bracket/.gitignore", "*.md\n.gitignore\n");

	memset(&result, 0, sizeof(result));
	cl_git_pass(git_status_foreach(repo, cb_status__single, &result));
	cl_assert_equal_i(2, result.count);
	cl_assert(result.status == GIT_STATUS_IGNORED);

	cl_git_pass(git_status_file(&status_flags, repo, FILE_WITH_BRACKET));
	cl_assert(status_flags == GIT_STATUS_IGNORED);

	/* don't ignore the file */

	cl_git_rewritefile("with_bracket/.gitignore", ".gitignore\n");

	memset(&result, 0, sizeof(result));
	cl_git_pass(git_status_foreach(repo, cb_status__single, &result));
	cl_assert_equal_i(2, result.count);
	cl_assert(result.status == GIT_STATUS_WT_NEW);

	cl_git_pass(git_status_file(&status_flags, repo, FILE_WITH_BRACKET));
	cl_assert(status_flags == GIT_STATUS_WT_NEW);

	/* add the file to the index */

	cl_git_pass(git_repository_index(&index, repo));
	cl_git_pass(git_index_add_bypath(index, FILE_WITH_BRACKET));
	cl_git_pass(git_index_write(index));

	memset(&result, 0, sizeof(result));
	cl_git_pass(git_status_foreach(repo, cb_status__single, &result));
	cl_assert_equal_i(2, result.count);
	cl_assert(result.status == GIT_STATUS_INDEX_NEW);

	cl_git_pass(git_status_file(&status_flags, repo, FILE_WITH_BRACKET));
	cl_assert(status_flags == GIT_STATUS_INDEX_NEW);

	/* Create file without bracket */

	cl_git_mkfile("with_bracket/" FILE_WITHOUT_BRACKET, "I have no bracket in my name!\n");

	cl_git_pass(git_status_file(&status_flags, repo, FILE_WITHOUT_BRACKET));
	cl_assert(status_flags == GIT_STATUS_WT_NEW);

	cl_git_fail_with(git_status_file(&status_flags, repo, "LICENSE\\[1\\].md"), GIT_ENOTFOUND);

	cl_git_pass(git_status_file(&status_flags, repo, FILE_WITH_BRACKET));
	cl_assert(status_flags == GIT_STATUS_INDEX_NEW);

	git_index_free(index);
	git_repository_free(repo);
}


// Source: worktree_init.c
// Lines 124-201
