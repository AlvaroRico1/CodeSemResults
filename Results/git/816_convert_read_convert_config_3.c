static int read_convert_config(const char *var, const char *value, void *cb)
{
	const char *key, *name;
	size_t namelen;
	struct convert_driver *drv;

	/*
	 * External conversion drivers are configured using
	 * "filter.<name>.variable".
	 */
	if (parse_config_key(var, "filter", &name, &namelen, &key) < 0 || !name)
		return 0;
	for (drv = user_convert; drv; drv = drv->next)
		if (!strncmp(drv->name, name, namelen) && !drv->name[namelen])
			break;
	if (!drv) {
		CALLOC_ARRAY(drv, 1);
		drv->name = xmemdupz(name, namelen);
		*user_convert_tail = drv;
		user_convert_tail = &(drv->next);
	}

	/*
	 * filter.<name>.smudge and filter.<name>.clean specifies
	 * the command line:
	 *
	 *	command-line
	 *
	 * The command-line will not be interpolated in any way.
	 */

	if (!strcmp("smudge", key))
		return git_config_string(&drv->smudge, var, value);

	if (!strcmp("clean", key))
		return git_config_string(&drv->clean, var, value);

	if (!strcmp("process", key))
		return git_config_string(&drv->process, var, value);

	if (!strcmp("required", key)) {
		drv->required = git_config_bool(var, value);
		return 0;
	}

	return 0;
}


// Source: convert.c
// Lines 1011-1057
