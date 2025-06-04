static int rename_config_entries_cb(
	const git_config_entry *entry,
	void *payload)
{
	int error = 0;
	struct rename_data *data = (struct rename_data *)payload;
	size_t base_len = git_str_len(data->name);

	if (base_len > 0 &&
		!(error = git_str_puts(data->name, entry->name + data->old_len)))
	{
		error = git_config_set_string(
			data->config, git_str_cstr(data->name), entry->value);

		git_str_truncate(data->name, base_len);
	}

	if (!error)
		error = git_config_delete_entry(data->config, entry->name);

	return error;
}


// Source: config.c
// Lines 1495-1516
