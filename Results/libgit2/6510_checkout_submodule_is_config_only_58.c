static bool submodule_is_config_only(
	checkout_data *data,
	const char *path)
{
	git_submodule *sm = NULL;
	unsigned int sm_loc = 0;
	bool rval = false;

	if (git_submodule_lookup(&sm, data->repo, path) < 0)
		return true;

	if (git_submodule_location(&sm_loc, sm) < 0 ||
		sm_loc == GIT_SUBMODULE_STATUS_IN_CONFIG)
		rval = true;

	git_submodule_free(sm);

	return rval;
}


// Source: checkout.c
// Lines 458-476
