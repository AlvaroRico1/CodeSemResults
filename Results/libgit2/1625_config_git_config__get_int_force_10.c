int git_config__get_int_force(
	const git_config *cfg, const char *key, int fallback_value)
{
	int32_t val = (int32_t)fallback_value;
	git_config_entry *entry;

	get_entry(&entry, cfg, key, false, GET_NO_ERRORS);

	if (entry && git_config_parse_int32(&val, entry->value) < 0)
		git_error_clear();

	git_config_entry_free(entry);
	return (int)val;
}


// Source: config.c
// Lines 973-986
