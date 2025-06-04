static struct remote *remote_get_1(const char *name,
				   const char *(*get_default)(struct branch *, int *))
{
	struct remote *ret;
	int name_given = 0;

	read_config();

	if (name)
		name_given = 1;
	else
		name = get_default(current_branch, &name_given);

	ret = make_remote(name, 0);
	if (valid_remote_nick(name) && have_git_dir()) {
		if (!valid_remote(ret))
			read_remotes_file(ret);
		if (!valid_remote(ret))
			read_branches_file(ret);
	}
	if (name_given && !valid_remote(ret))
		add_url_alias(ret, name);
	if (!valid_remote(ret))
		return NULL;
	return ret;
}


// Source: remote.c
// Lines 526-551
