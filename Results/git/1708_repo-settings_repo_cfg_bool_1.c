static void repo_cfg_bool(struct repository *r, const char *key, int *dest,
			  int def)
{
	if (repo_config_get_bool(r, key, dest))
		*dest = def;
}


// Source: repo-settings.c
// Lines 6-11
