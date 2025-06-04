make_label(const char *label, char **cause)
{
	char		**paths, *path, *base;
	u_int		  i, n;
	struct stat	  sb;
	uid_t		  uid;

	*cause = NULL;
	if (label == NULL)
		label = "default";
	uid = getuid();

	expand_paths(TMUX_SOCK, &paths, &n, 1);
	if (n == 0) {
		xasprintf(cause, "no suitable socket path");
		return (NULL);
	}
	path = paths[0]; /* can only have one socket! */
	for (i = 1; i < n; i++)
		free(paths[i]);
	free(paths);

	xasprintf(&base, "%s/tmux-%ld", path, (long)uid);
	free(path);
	if (mkdir(base, S_IRWXU) != 0 && errno != EEXIST) {
		xasprintf(cause, "couldn't create directory %s (%s)", base,
		    strerror(errno));
		goto fail;
	}
	if (lstat(base, &sb) != 0) {
		xasprintf(cause, "couldn't read directory %s (%s)", base,
		    strerror(errno));
		goto fail;
	}
	if (!S_ISDIR(sb.st_mode)) {
		xasprintf(cause, "%s is not a directory", base);
		goto fail;
	}
	if (sb.st_uid != uid || (sb.st_mode & S_IRWXO) != 0) {
		xasprintf(cause, "directory %s has unsafe permissions", base);
		goto fail;
	}
	xasprintf(&path, "%s/%s", base, label);
	free(base);
	return (path);

fail:
	free(base);
	return (NULL);
}


// Source: tmux.c
// Lines 187-236
