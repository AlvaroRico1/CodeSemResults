static void reparent_cb(const char *name,
			const char *old_cwd,
			const char *new_cwd,
			void *data)
{
	char **path = data;
	char *tmp = *path;

	if (!tmp)
		return;

	*path = reparent_relative_path(old_cwd, new_cwd, tmp);
	free(tmp);

	if (name) {
		trace_printf_key(&trace_setup_key,
				 "setup: reparent %s to '%s'",
				 name, *path);
	}
}


// Source: chdir-notify.c
// Lines 25-44
