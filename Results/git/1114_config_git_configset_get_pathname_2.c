int git_configset_get_pathname(struct config_set *cs, const char *key, const char **dest)
{
	const char *value;
	if (!git_configset_get_value(cs, key, &value))
		return git_config_pathname(dest, key, value);
	else
		return 1;
}


// Source: config.c
// Lines 2242-2249
