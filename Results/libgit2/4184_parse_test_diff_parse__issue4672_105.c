void test_diff_parse__issue4672(void)
{
	const char *text = "diff --git a/a b/a\n"
	"index 7f129fd..af431f2 100644\n"
	"--- a/a\n"
	"+++ b/a\n"
	"@@ -3 +3 @@\n"
	"-a contents 2\n"
	"+a contents\n";

	git_diff *diff;
	git_patch *patch;
	const git_diff_hunk *hunk;
	size_t n, l = 0;

	cl_git_pass(git_diff_from_buffer(&diff, text, strlen(text)));
	cl_git_pass(git_patch_from_diff(&patch, diff, 0));
	cl_git_pass(git_patch_get_hunk(&hunk, &n, patch, 0));

	cl_git_assert_lineinfo(3, -1, 1, patch, 0, l++);
	cl_git_assert_lineinfo(-1, 3, 1, patch, 0, l++);

	cl_assert_equal_i(n, l);

	git_patch_free(patch);
	git_diff_free(diff);
}


// Source: parse.c
// Lines 360-386
