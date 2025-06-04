int git_configset_get_string_tmp(struct config_set *cs, const char *key,
				 const char **dest)
{
	const char *value;
	if (!git_configset_get_value(cs, key, &value)) {
		if (!value)
			return config_error_nonbool(key);
		*dest = value;
		return 0;
	} else {
		return 1;
	}


// Source: config.c
// Lines 2175-2186
