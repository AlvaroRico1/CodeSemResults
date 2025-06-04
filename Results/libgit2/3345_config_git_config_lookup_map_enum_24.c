int git_config_lookup_map_enum(git_configmap_t *type_out, const char **str_out,
			       const git_configmap *maps, size_t map_n, int enum_val)
{
	size_t i;

	for (i = 0; i < map_n; i++) {
		const git_configmap *m = &maps[i];

		if (m->map_value != enum_val)
			continue;

		*type_out = m->type;
		*str_out = m->str_match;
		return 0;
	}


// Source: config.c
// Lines 1332-1346
