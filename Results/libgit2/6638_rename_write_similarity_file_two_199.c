static void write_similarity_file_two(const char *filename, size_t b_lines)
{
	git_str contents = GIT_STR_INIT;
	size_t i;

	for (i = 0; i < b_lines; i++)
		git_str_printf(&contents, "%02d - bbbbb\r\n", (int)(i+1));

	for (i = b_lines; i < 50; i++)
		git_str_printf(&contents, "%02d - aaaaa%s", (int)(i+1), (i == 49 ? "" : "\r\n"));

	cl_git_pass(
		git_futils_writebuffer(&contents, filename, O_RDWR|O_CREAT, 0777));

	git_str_dispose(&contents);
}


// Source: rename.c
// Lines 1031-1046
