static void setup_pager_env(struct strvec *env)
{
	const char **argv;
	int i;
	char *pager_env = xstrdup(PAGER_ENV);
	int n = split_cmdline(pager_env, &argv);

	if (n < 0)
		die("malformed build-time PAGER_ENV: %s",
			split_cmdline_strerror(n));

	for (i = 0; i < n; i++) {
		char *cp = strchr(argv[i], '=');

		if (!cp)
			die("malformed build-time PAGER_ENV");

		*cp = '\0';
		if (!getenv(argv[i])) {
			*cp = '=';
			strvec_push(env, argv[i]);
		}
	}


// Source: pager.c
// Lines 71-93
