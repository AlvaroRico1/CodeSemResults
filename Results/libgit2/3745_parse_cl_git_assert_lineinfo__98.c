static void cl_git_assert_lineinfo_(int old_lineno, int new_lineno, int num_lines, git_patch *patch, size_t hunk_idx, size_t line_idx, const char *file, const char *func, int lineno)
{
	const git_diff_line *line;

	cl_git_expect(git_patch_get_line_in_hunk(&line, patch, hunk_idx, line_idx), 0, file, func, lineno);
	cl_assert_equal_i_src(old_lineno, line->old_lineno, file, func, lineno);
	cl_assert_equal_i_src(new_lineno, line->new_lineno, file, func, lineno);
	cl_assert_equal_i_src(num_lines, line->num_lines, file, func, lineno);
}


// Source: parse.c
// Lines 346-354
