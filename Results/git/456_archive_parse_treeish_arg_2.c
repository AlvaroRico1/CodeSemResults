static void parse_treeish_arg(const char **argv,
		struct archiver_args *ar_args, const char *prefix,
		int remote)
{
	const char *name = argv[0];
	const struct object_id *commit_oid;
	time_t archive_time;
	struct tree *tree;
	const struct commit *commit;
	struct object_id oid;
	char *ref = NULL;

	/* Remotes are only allowed to fetch actual refs */
	if (remote && !remote_allow_unreachable) {
		const char *colon = strchrnul(name, ':');
		int refnamelen = colon - name;

		if (!dwim_ref(name, refnamelen, &oid, &ref, 0))
			die(_("no such ref: %.*s"), refnamelen, name);
	} else {
		dwim_ref(name, strlen(name), &oid, &ref, 0);
	}

	if (get_oid(name, &oid))
		die(_("not a valid object name: %s"), name);

	commit = lookup_commit_reference_gently(ar_args->repo, &oid, 1);
	if (commit) {
		commit_oid = &commit->object.oid;
		archive_time = commit->date;
	} else {
		commit_oid = NULL;
		archive_time = time(NULL);
	}

	tree = parse_tree_indirect(&oid);
	if (tree == NULL)
		die(_("not a tree object: %s"), oid_to_hex(&oid));

	if (prefix) {
		struct object_id tree_oid;
		unsigned short mode;
		int err;

		err = get_tree_entry(ar_args->repo,
				     &tree->object.oid,
				     prefix, &tree_oid,
				     &mode);
		if (err || !S_ISDIR(mode))
			die(_("current working directory is untracked"));

		tree = parse_tree_indirect(&tree_oid);
	}


// Source: archive.c
// Lines 432-484
