int git_config__get_bool_force(
	const git_config *cfg, const char *key, int fallback_value)
{
	int val = fallback_value;
	git_config_entry *entry;

	get_entry(&entry, cfg, key, false, GET_NO_ERRORS);

	if (entry && git_config_parse_bool(&val, entry->value) < 0)
		git_error_clear();

	git_config_entry_free(entry);
	return val;
}


// Source: config.c
// Lines 958-971
