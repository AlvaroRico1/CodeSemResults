void test_diff_parse__add_delete_no_index(void)
{
	const char *content =
	    "diff --git a/file.txt b/file.txt\n"
	    "new file mode 100644\n"
	    "--- /dev/null\n"
	    "+++ b/file.txt\n"
	    "@@ -0,0 +1,2 @@\n"
	    "+one\n"
	    "+two\n"
	    "diff --git a/otherfile.txt b/otherfile.txt\n"
	    "deleted file mode 100644\n"
	    "--- a/otherfile.txt\n"
	    "+++ /dev/null\n"
	    "@@ -1,1 +0,0 @@\n"
	    "-three\n";
	git_diff *diff;

	cl_git_pass(git_diff_from_buffer(
		&diff, content, strlen(content)));
	git_diff_free(diff);
}


// Source: parse.c
// Lines 110-131
