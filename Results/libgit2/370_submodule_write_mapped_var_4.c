static int write_mapped_var(git_repository *repo, const char *name, git_configmap *maps, size_t nmaps, const char *var, int ival)
{
	git_configmap_t type;
	const char *val;

	if (git_config_lookup_map_enum(&type, &val, maps, nmaps, ival) < 0) {
		git_error_set(GIT_ERROR_SUBMODULE, "invalid value for %s", var);
		return -1;
	}

	if (type == GIT_CONFIGMAP_TRUE)
		val = "true";

	return write_var(repo, name, var, val);
}


// Source: submodule.c
// Lines 1109-1123
