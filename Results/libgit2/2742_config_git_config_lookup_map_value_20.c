int git_config_lookup_map_value(
	int *out,
	const git_configmap *maps,
	size_t map_n,
	const char *value)
{
	size_t i;

	for (i = 0; i < map_n; ++i) {
		const git_configmap *m = maps + i;

		switch (m->type) {
		case GIT_CONFIGMAP_FALSE:
		case GIT_CONFIGMAP_TRUE: {
			int bool_val;

			if (git_config_parse_bool(&bool_val, value) == 0 &&
				bool_val == (int)m->type) {
				*out = m->map_value;
				return 0;
			}
			break;
		}

		case GIT_CONFIGMAP_INT32:
			if (git_config_parse_int32(out, value) == 0)
				return 0;
			break;

		case GIT_CONFIGMAP_STRING:
			if (value && strcasecmp(value, m->str_match) == 0) {
				*out = m->map_value;
				return 0;
			}
			break;
		}
	}


// Source: config.c
// Lines 1290-1326
