find_tmp_path(char *buffer, size_t length)
{
#ifndef _WIN32
	static const size_t var_count = 5;
	static const char *env_vars[] = {
		"CLAR_TMP", "TMPDIR", "TMP", "TEMP", "USERPROFILE"
 	};

 	size_t i;

	for (i = 0; i < var_count; ++i) {
		const char *env = getenv(env_vars[i]);
		if (!env)
			continue;

		if (is_valid_tmp_path(env)) {
#ifdef __APPLE__
			if (length >= PATH_MAX && realpath(env, buffer) != NULL)
				return 0;
#endif
			strncpy(buffer, env, length - 1);
			buffer[length - 1] = '\0';
			return 0;
		}
	}


// Source: sandbox.h
// Lines 22-46
