static int handle_config(const char *key, const char *value, void *cb)
{
	const char *name;
	size_t namelen;
	const char *subkey;
	struct remote *remote;
	struct branch *branch;
	if (parse_config_key(key, "branch", &name, &namelen, &subkey) >= 0) {
		if (!name)
			return 0;
		branch = make_branch(name, namelen);
		if (!strcmp(subkey, "remote")) {
			return git_config_string(&branch->remote_name, key, value);
		} else if (!strcmp(subkey, "pushremote")) {
			return git_config_string(&branch->pushremote_name, key, value);
		} else if (!strcmp(subkey, "merge")) {
			if (!value)
				return config_error_nonbool(key);
			add_merge(branch, xstrdup(value));
		}
		return 0;
	}
	if (parse_config_key(key, "url", &name, &namelen, &subkey) >= 0) {
		struct rewrite *rewrite;
		if (!name)
			return 0;
		if (!strcmp(subkey, "insteadof")) {
			if (!value)
				return config_error_nonbool(key);
			rewrite = make_rewrite(&rewrites, name, namelen);
			add_instead_of(rewrite, xstrdup(value));
		} else if (!strcmp(subkey, "pushinsteadof")) {
			if (!value)
				return config_error_nonbool(key);
			rewrite = make_rewrite(&rewrites_push, name, namelen);
			add_instead_of(rewrite, xstrdup(value));
		}
	}

	if (parse_config_key(key, "remote", &name, &namelen, &subkey) < 0)
		return 0;

	/* Handle remote.* variables */
	if (!name && !strcmp(subkey, "pushdefault"))
		return git_config_string(&pushremote_name, key, value);

	if (!name)
		return 0;
	/* Handle remote.<name>.* variables */
	if (*name == '/') {
		warning(_("config remote shorthand cannot begin with '/': %s"),
			name);
		return 0;
	}
	remote = make_remote(name, namelen);
	remote->origin = REMOTE_CONFIG;
	if (current_config_scope() == CONFIG_SCOPE_LOCAL ||
	    current_config_scope() == CONFIG_SCOPE_WORKTREE)
		remote->configured_in_repo = 1;
	if (!strcmp(subkey, "mirror"))
		remote->mirror = git_config_bool(key, value);
	else if (!strcmp(subkey, "skipdefaultupdate"))
		remote->skip_default_update = git_config_bool(key, value);
	else if (!strcmp(subkey, "skipfetchall"))
		remote->skip_default_update = git_config_bool(key, value);
	else if (!strcmp(subkey, "prune"))
		remote->prune = git_config_bool(key, value);
	else if (!strcmp(subkey, "prunetags"))
		remote->prune_tags = git_config_bool(key, value);
	else if (!strcmp(subkey, "url")) {
		const char *v;
		if (git_config_string(&v, key, value))
			return -1;
		add_url(remote, v);
	} else if (!strcmp(subkey, "pushurl")) {
		const char *v;
		if (git_config_string(&v, key, value))
			return -1;
		add_pushurl(remote, v);
	} else if (!strcmp(subkey, "push")) {
		const char *v;
		if (git_config_string(&v, key, value))
			return -1;
		refspec_append(&remote->push, v);
		free((char *)v);
	} else if (!strcmp(subkey, "fetch")) {
		const char *v;
		if (git_config_string(&v, key, value))
			return -1;
		refspec_append(&remote->fetch, v);
		free((char *)v);
	} else if (!strcmp(subkey, "receivepack")) {
		const char *v;
		if (git_config_string(&v, key, value))
			return -1;
		if (!remote->receivepack)
			remote->receivepack = v;
		else
			error(_("more than one receivepack given, using the first"));
	} else if (!strcmp(subkey, "uploadpack")) {
		const char *v;
		if (git_config_string(&v, key, value))
			return -1;
		if (!remote->uploadpack)
			remote->uploadpack = v;
		else
			error(_("more than one uploadpack given, using the first"));
	} else if (!strcmp(subkey, "tagopt")) {
		if (!strcmp(value, "--no-tags"))
			remote->fetch_tags = -1;
		else if (!strcmp(value, "--tags"))
			remote->fetch_tags = 2;
	} else if (!strcmp(subkey, "proxy")) {
		return git_config_string((const char **)&remote->http_proxy,
					 key, value);
	} else if (!strcmp(subkey, "proxyauthmethod")) {
		return git_config_string((const char **)&remote->http_proxy_authmethod,
					 key, value);
	} else if (!strcmp(subkey, "vcs")) {
		return git_config_string(&remote->foreign_vcs, key, value);
	}
	return 0;
}


// Source: remote.c
// Lines 301-423
