void test_diff_parse__empty_file(void)
{
	const char *content =
	    "---\n"
	    " file | 0\n"
	    " 1 file changed, 0 insertions(+), 0 deletions(-)\n"
	    " created mode 100644 file\n"
	    "\n"
	    "diff --git a/file b/file\n"
	    "new file mode 100644\n"
	    "index 0000000..e69de29\n"
	    "-- \n"
	    "2.20.1\n";
	git_diff *diff;

	cl_git_pass(git_diff_from_buffer(
		&diff, content, strlen(content)));
	git_diff_free(diff);
}


// Source: parse.c
// Lines 80-98
