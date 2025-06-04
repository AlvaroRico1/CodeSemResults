void test_commit_parse__header(void)
{
	git_oid oid;

	parse_test_case *testcase;
	for (testcase = passing_header_cases; testcase->line != NULL; testcase++)
	{
		const char *line = testcase->line;
		const char *line_end = line + strlen(line);

		cl_git_pass(git_oid__parse(&oid, &line, line_end, testcase->header));
		cl_assert(line == line_end);
	}


// Source: parse.c
// Lines 49-61
