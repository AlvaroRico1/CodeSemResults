static int fsck_gitmodules_fn(const char *var, const char *value, void *vdata)
{
	struct fsck_gitmodules_data *data = vdata;
	const char *subsection, *key;
	size_t subsection_len;
	char *name;

	if (parse_config_key(var, "submodule", &subsection, &subsection_len, &key) < 0 ||
	    !subsection)
		return 0;

	name = xmemdupz(subsection, subsection_len);
	if (check_submodule_name(name) < 0)
		data->ret |= report(data->options,
				    data->oid, OBJ_BLOB,
				    FSCK_MSG_GITMODULES_NAME,
				    "disallowed submodule name: %s",
				    name);
	if (!strcmp(key, "url") && value &&
	    check_submodule_url(value) < 0)
		data->ret |= report(data->options,
				    data->oid, OBJ_BLOB,
				    FSCK_MSG_GITMODULES_URL,
				    "disallowed submodule url: %s",
				    value);
	if (!strcmp(key, "path") && value &&
	    looks_like_command_line_option(value))
		data->ret |= report(data->options,
				    data->oid, OBJ_BLOB,
				    FSCK_MSG_GITMODULES_PATH,
				    "disallowed submodule path: %s",
				    value);
	if (!strcmp(key, "update") && value &&
	    parse_submodule_update_type(value) == SM_UPDATE_COMMAND)
		data->ret |= report(data->options, data->oid, OBJ_BLOB,
				    FSCK_MSG_GITMODULES_UPDATE,
				    "disallowed submodule update setting: %s",
				    value);
	free(name);

	return 0;
}


// Source: fsck.c
// Lines 1127-1168
