void test_diff_patch__can_strip_bad_utf8(void)
{
	const char *a = "A " UTF8_HUNK_HEADER
		"  B\n"
		"  C\n"
		"  D\n"
		"  E\n"
		"  F\n"
		"  G\n"
		"  H\n"
		"  I\n"
		"  J\n"
		"  K\n"
		"L  " UTF8_HUNK_HEADER
		"  M\n"
		"  N\n"
		"  O\n"
		"  P\n"
		"  Q\n"
		"  R\n"
		"  S\n"
		"  T\n"
		"  U\n"
		"  V\n";

	const char *b = "A " UTF8_HUNK_HEADER
		"  B\n"
		"  C\n"
		"  D\n"
		"  E modified\n"
		"  F\n"
		"  G\n"
		"  H\n"
		"  I\n"
		"  J\n"
		"  K\n"
		"L  " UTF8_HUNK_HEADER
		"  M\n"
		"  N\n"
		"  O\n"
		"  P modified\n"
		"  Q\n"
		"  R\n"
		"  S\n"
		"  T\n"
		"  U\n"
		"  V\n";

	const char *expected = "diff --git a/file b/file\n"
		"index d0647c4..7827ce5 100644\n"
		"--- a/file\n"
		"+++ b/file\n"
		"@@ -2,7 +2,7 @@ A " UTF8_TRUNCATED_A_HUNK_HEADER
		"   B\n"
		"   C\n"
		"   D\n"
		"-  E\n"
		"+  E modified\n"
		"   F\n"
		"   G\n"
		"   H\n"
		"@@ -13,7 +13,7 @@ L  " UTF8_TRUNCATED_L_HUNK_HEADER
		"   M\n"
		"   N\n"
		"   O\n"
		"-  P\n"
		"+  P modified\n"
		"   Q\n"
		"   R\n"
		"   S\n";

	git_diff_options opts;
	git_patch *patch;
	git_buf buf = GIT_BUF_INIT;

	cl_git_pass(git_diff_options_init(&opts, GIT_DIFF_OPTIONS_VERSION));

	cl_git_pass(git_patch_from_buffers(&patch, a, strlen(a), NULL, b, strlen(b), NULL, &opts));
	cl_git_pass(git_patch_to_buf(&buf, patch));

	cl_assert_equal_s(expected, buf.ptr);

	git_patch_free(patch);
	git_buf_dispose(&buf);
}


// Source: patch.c
// Lines 619-703
