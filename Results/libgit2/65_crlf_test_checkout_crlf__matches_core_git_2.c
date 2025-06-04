void test_checkout_crlf__matches_core_git(void)
{
	const char *autocrlf[] = { "true", "false", "input", NULL };
	const char *attrs[] = { "", "-crlf", "-text", "eol=crlf", "eol=lf",
		"text", "text eol=crlf", "text eol=lf",
		"text=auto", "text=auto eol=crlf", "text=auto eol=lf",
		NULL };
	const char **a, **b;

	for (a = autocrlf; *a; a++) {
		for (b = attrs; *b; b++) {
			empty_workdir("crlf");
			test_checkout(*a, *b);
		}
	}
}


// Source: crlf.c
// Lines 203-218
