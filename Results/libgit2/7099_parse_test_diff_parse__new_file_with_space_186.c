void test_diff_parse__new_file_with_space(void)
{
	const char *content = PATCH_ORIGINAL_NEW_FILE_WITH_SPACE;
	git_patch *patch;
	git_diff *diff;

	cl_git_pass(git_diff_from_buffer(&diff, content, strlen(content)));
	cl_git_pass(git_patch_from_diff((git_patch **) &patch, diff, 0));

	cl_assert_equal_p(patch->diff_opts.old_prefix, NULL);
	cl_assert_equal_p(patch->delta->old_file.path, NULL);
	cl_assert_equal_s(patch->diff_opts.new_prefix, "b/");
	cl_assert_equal_s(patch->delta->new_file.path, "sp ace.txt");

	git_patch_free(patch);
	git_diff_free(diff);
}


// Source: parse.c
// Lines 416-432
