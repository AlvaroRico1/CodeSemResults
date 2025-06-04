static struct passwd *xgetpwuid_self(int *is_bogus)
{
	struct passwd *pw;

	errno = 0;
	pw = getpwuid(getuid());
	if (!pw) {
		static struct passwd fallback;
		fallback.pw_name = "unknown";
#ifndef NO_GECOS_IN_PWENT
		fallback.pw_gecos = "Unknown";
#endif
		pw = &fallback;
		if (is_bogus)
			*is_bogus = 1;
	}


// Source: ident.c
// Lines 36-51
