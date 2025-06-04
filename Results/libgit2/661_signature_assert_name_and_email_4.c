static void assert_name_and_email(
	const char *expected_name,
	const char *expected_email,
	const char *name,
	const char *email)
{
	git_signature *sign;

	cl_git_pass(git_signature_new(&sign, name, email, 1234567890, 60));
	cl_assert_equal_s(expected_name, sign->name);
	cl_assert_equal_s(expected_email, sign->email);

	git_signature_free(sign);
}


// Source: signature.c
// Lines 17-30
