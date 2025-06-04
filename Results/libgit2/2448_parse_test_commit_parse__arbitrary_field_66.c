void test_commit_parse__arbitrary_field(void)
{
	git_commit *commit;
	git_buf buf = GIT_BUF_INIT;
	const char *gpgsig = "-----BEGIN PGP SIGNATURE-----\n\
Version: GnuPG v1.4.12 (Darwin)\n\
\n\
iQIcBAABAgAGBQJQ+FMIAAoJEH+LfPdZDSs1e3EQAJMjhqjWF+WkGLHju7pTw2al\n\
o6IoMAhv0Z/LHlWhzBd9e7JeCnanRt12bAU7yvYp9+Z+z+dbwqLwDoFp8LVuigl8\n\
JGLcnwiUW3rSvhjdCp9irdb4+bhKUnKUzSdsR2CK4/hC0N2i/HOvMYX+BRsvqweq\n\
AsAkA6dAWh+gAfedrBUkCTGhlNYoetjdakWqlGL1TiKAefEZrtA1TpPkGn92vbLq\n\
SphFRUY9hVn1ZBWrT3hEpvAIcZag3rTOiRVT1X1flj8B2vGCEr3RrcwOIZikpdaW\n\
who/X3xh/DGbI2RbuxmmJpxxP/8dsVchRJJzBwG+yhwU/iN3MlV2c5D69tls/Dok\n\
6VbyU4lm/ae0y3yR83D9dUlkycOnmmlBAHKIZ9qUts9X7mWJf0+yy2QxJVpjaTGG\n\
cmnQKKPeNIhGJk2ENnnnzjEve7L7YJQF6itbx5VCOcsGh3Ocb3YR7DMdWjt7f8pu\n\
c6j+q1rP7EpE2afUN/geSlp5i3x8aXZPDj67jImbVCE/Q1X9voCtyzGJH7MXR0N9\n\
ZpRF8yzveRfMH8bwAJjSOGAFF5XkcR/RNY95o+J+QcgBLdX48h+ZdNmUf6jqlu3J\n\
7KmTXXQcOVpN6dD3CmRFsbjq+x6RHwa8u1iGn+oIkX908r97ckfB/kHKH7ZdXIJc\n\
cpxtDQQMGYFpXK/71stq\n\
=ozeK\n\
-----END PGP SIGNATURE-----";

	cl_git_pass(parse_commit(&commit, passing_commit_cases[4]));

	cl_git_pass(git_commit_header_field(&buf, commit, "tree"));
	cl_assert_equal_s("6b79e22d69bf46e289df0345a14ca059dfc9bdf6", buf.ptr);
	git_buf_dispose(&buf);

	cl_git_pass(git_commit_header_field(&buf, commit, "parent"));
	cl_assert_equal_s("34734e478d6cf50c27c9d69026d93974d052c454", buf.ptr);
	git_buf_dispose(&buf);

	cl_git_pass(git_commit_header_field(&buf, commit, "gpgsig"));
	cl_assert_equal_s(gpgsig, buf.ptr);
	git_buf_dispose(&buf);

	cl_git_fail_with(GIT_ENOTFOUND, git_commit_header_field(&buf, commit, "awesomeness"));
	cl_git_fail_with(GIT_ENOTFOUND, git_commit_header_field(&buf, commit, "par"));

	git_commit__free(commit);
	cl_git_pass(parse_commit(&commit, passing_commit_cases[0]));

	cl_git_pass(git_commit_header_field(&buf, commit, "committer"));
	cl_assert_equal_s("Vicent Marti <tanoku@gmail.com> 1273848544 +0200", buf.ptr);

	git_buf_dispose(&buf);
	git_commit__free(commit);
}


// Source: parse.c
// Lines 422-469
