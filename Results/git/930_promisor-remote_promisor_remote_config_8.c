static int promisor_remote_config(const char *var, const char *value, void *data)
{
	struct promisor_remote_config *config = data;
	const char *name;
	size_t namelen;
	const char *subkey;

	if (parse_config_key(var, "remote", &name, &namelen, &subkey) < 0)
		return 0;

	if (!strcmp(subkey, "promisor")) {
		char *remote_name;

		if (!git_config_bool(var, value))
			return 0;

		remote_name = xmemdupz(name, namelen);

		if (!promisor_remote_lookup(config, remote_name, NULL))
			promisor_remote_new(config, remote_name);

		free(remote_name);
		return 0;
	}
	if (!strcmp(subkey, "partialclonefilter")) {
		struct promisor_remote *r;
		char *remote_name = xmemdupz(name, namelen);

		r = promisor_remote_lookup(config, remote_name, NULL);
		if (!r)
			r = promisor_remote_new(config, remote_name);

		free(remote_name);

		if (!r)
			return 0;

		return git_config_string(&r->partial_clone_filter, var, value);
	}

	return 0;
}


// Source: promisor-remote.c
// Lines 99-140
