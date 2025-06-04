int cmd_update_ref(int argc, const char **argv, const char *prefix)
{
	const char *refname, *oldval;
	struct object_id oid, oldoid;
	int delete = 0, no_deref = 0, read_stdin = 0, end_null = 0;
	int create_reflog = 0;
	struct option options[] = {
		OPT_STRING( 'm', NULL, &msg, N_("reason"), N_("reason of the update")),
		OPT_BOOL('d', NULL, &delete, N_("delete the reference")),
		OPT_BOOL( 0 , "no-deref", &no_deref,
					N_("update <refname> not the one it points to")),
		OPT_BOOL('z', NULL, &end_null, N_("stdin has NUL-terminated arguments")),
		OPT_BOOL( 0 , "stdin", &read_stdin, N_("read updates from stdin")),
		OPT_BOOL( 0 , "create-reflog", &create_reflog, N_("create a reflog")),
		OPT_END(),
	};

	git_config(git_default_config, NULL);
	argc = parse_options(argc, argv, prefix, options, git_update_ref_usage,
			     0);
	if (msg && !*msg)
		die("Refusing to perform update with empty message.");

	create_reflog_flag = create_reflog ? REF_FORCE_CREATE_REFLOG : 0;

	if (no_deref) {
		default_flags = REF_NO_DEREF;
		update_flags = default_flags;
	}

	if (read_stdin) {
		if (delete || argc > 0)
			usage_with_options(git_update_ref_usage, options);
		if (end_null)
			line_termination = '\0';
		update_refs_stdin();
		return 0;
	}

	if (end_null)
		usage_with_options(git_update_ref_usage, options);

	if (delete) {
		if (argc < 1 || argc > 2)
			usage_with_options(git_update_ref_usage, options);
		refname = argv[0];
		oldval = argv[1];
	} else {
		const char *value;
		if (argc < 2 || argc > 3)
			usage_with_options(git_update_ref_usage, options);
		refname = argv[0];
		value = argv[1];
		oldval = argv[2];
		if (get_oid(value, &oid))
			die("%s: not a valid SHA1", value);
	}

	if (oldval) {
		if (!*oldval)
			/*
			 * The empty string implies that the reference
			 * must not already exist:
			 */
			oidclr(&oldoid);
		else if (get_oid(oldval, &oldoid))
			die("%s: not a valid old SHA1", oldval);
	}

	if (delete)
		/*
		 * For purposes of backwards compatibility, we treat
		 * NULL_SHA1 as "don't care" here:
		 */
		return delete_ref(msg, refname,
				  (oldval && !is_null_oid(&oldoid)) ? &oldoid : NULL,
				  default_flags);
	else
		return update_ref(msg, refname, &oid, oldval ? &oldoid : NULL,
				  default_flags | create_reflog_flag,
				  UPDATE_REFS_DIE_ON_ERR);
}


// Source: update-ref.c
// Lines 498-579
