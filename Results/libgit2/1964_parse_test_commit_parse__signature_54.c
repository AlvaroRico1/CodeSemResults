void test_commit_parse__signature(void)
{
	passing_signature_test_case *passcase;
	failing_signature_test_case *failcase;

	for (passcase = passing_signature_cases; passcase->string != NULL; passcase++)
	{
		const char *str = passcase->string;
		size_t len = strlen(passcase->string);
		struct git_signature person = {0};

		cl_git_pass(git_signature__parse(&person, &str, str + len, passcase->header, '\n'));
		cl_assert_equal_s(passcase->name, person.name);
		cl_assert_equal_s(passcase->email, person.email);
		cl_assert_equal_i((int)passcase->time, (int)person.when.time);
		cl_assert_equal_i(passcase->offset, person.when.offset);
		git__free(person.name); git__free(person.email);
	}

	for (failcase = failing_signature_cases; failcase->string != NULL; failcase++)
	{
		const char *str = failcase->string;
		size_t len = strlen(failcase->string);
		git_signature person = {0};
		cl_git_fail(git_signature__parse(&person, &str, str + len, failcase->header, '\n'));
		git__free(person.name); git__free(person.email);
	}
}


// Source: parse.c
// Lines 144-171
