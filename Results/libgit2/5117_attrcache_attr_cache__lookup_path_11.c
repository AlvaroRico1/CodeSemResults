static int attr_cache__lookup_path(
	char **out, git_config *cfg, const char *key, const char *fallback)
{
	git_str buf = GIT_STR_INIT;
	int error;
	git_config_entry *entry = NULL;

	*out = NULL;

	if ((error = git_config__lookup_entry(&entry, cfg, key, false)) < 0)
		return error;

	if (entry) {
		const char *cfgval = entry->value;

		/* expand leading ~/ as needed */
		if (cfgval && cfgval[0] == '~' && cfgval[1] == '/') {
			if (! (error = git_sysdir_expand_global_file(&buf, &cfgval[2])))
				*out = git_str_detach(&buf);
		} else if (cfgval) {
			*out = git__strdup(cfgval);
		}
	}


// Source: attrcache.c
// Lines 286-308
