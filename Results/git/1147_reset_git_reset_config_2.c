static int git_reset_config(const char *var, const char *value, void *cb)
{
	if (!strcmp(var, "submodule.recurse"))
		return git_default_submodule_config(var, value, cb);

	return git_default_config(var, value, cb);
}


// Source: reset.c
// Lines 284-290
