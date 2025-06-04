int cmd_rev_list(int argc, const char **argv, const char *prefix)
{
	struct rev_info revs;
	struct rev_list_info info;
	struct setup_revision_opt s_r_opt = {
		.allow_exclude_promisor_objects = 1,
	};
	int i;
	int bisect_list = 0;
	int bisect_show_vars = 0;
	int bisect_find_all = 0;
	int use_bitmap_index = 0;
	int filter_provided_objects = 0;
	const char *show_progress = NULL;

	if (argc == 2 && !strcmp(argv[1], "-h"))
		usage(rev_list_usage);

	git_config(git_default_config, NULL);
	repo_init_revisions(the_repository, &revs, prefix);
	revs.abbrev = DEFAULT_ABBREV;
	revs.commit_format = CMIT_FMT_UNSPECIFIED;
	revs.include_header = 1;

	/*
	 * Scan the argument list before invoking setup_revisions(), so that we
	 * know if fetch_if_missing needs to be set to 0.
	 *
	 * "--exclude-promisor-objects" acts as a pre-filter on missing objects
	 * by not crossing the boundary from realized objects to promisor
	 * objects.
	 *
	 * Let "--missing" to conditionally set fetch_if_missing.
	 */
	for (i = 1; i < argc; i++) {
		const char *arg = argv[i];
		if (!strcmp(arg, "--exclude-promisor-objects")) {
			fetch_if_missing = 0;
			revs.exclude_promisor_objects = 1;
			break;
		}
	}
	for (i = 1; i < argc; i++) {
		const char *arg = argv[i];
		if (skip_prefix(arg, "--missing=", &arg)) {
			if (revs.exclude_promisor_objects)
				die(_("cannot combine --exclude-promisor-objects and --missing"));
			if (parse_missing_action_value(arg))
				break;
		}
	}

	if (arg_missing_action)
		revs.do_not_die_on_missing_tree = 1;

	argc = setup_revisions(argc, argv, &revs, &s_r_opt);

	memset(&info, 0, sizeof(info));
	info.revs = &revs;
	if (revs.bisect)
		bisect_list = 1;

	if (revs.diffopt.flags.quick)
		info.flags |= REV_LIST_QUIET;
	for (i = 1 ; i < argc; i++) {
		const char *arg = argv[i];

		if (!strcmp(arg, "--header")) {
			revs.verbose_header = 1;
			continue;
		}
		if (!strcmp(arg, "--timestamp")) {
			info.show_timestamp = 1;
			continue;
		}
		if (!strcmp(arg, "--bisect")) {
			bisect_list = 1;
			continue;
		}
		if (!strcmp(arg, "--bisect-all")) {
			bisect_list = 1;
			bisect_find_all = 1;
			info.flags |= BISECT_SHOW_ALL;
			revs.show_decorations = 1;
			continue;
		}
		if (!strcmp(arg, "--bisect-vars")) {
			bisect_list = 1;
			bisect_show_vars = 1;
			continue;
		}
		if (!strcmp(arg, "--use-bitmap-index")) {
			use_bitmap_index = 1;
			continue;
		}
		if (!strcmp(arg, "--test-bitmap")) {
			test_bitmap_walk(&revs);
			return 0;
		}
		if (skip_prefix(arg, "--progress=", &arg)) {
			show_progress = arg;
			continue;
		}

		if (skip_prefix(arg, ("--" CL_ARG__FILTER "="), &arg)) {
			parse_list_objects_filter(&filter_options, arg);
			if (filter_options.choice && !revs.blob_objects)
				die(_("object filtering requires --objects"));
			continue;
		}
		if (!strcmp(arg, ("--no-" CL_ARG__FILTER))) {
			list_objects_filter_set_no_filter(&filter_options);
			continue;
		}
		if (!strcmp(arg, "--filter-provided-objects")) {
			filter_provided_objects = 1;
			continue;
		}
		if (!strcmp(arg, "--filter-print-omitted")) {
			arg_print_omitted = 1;
			continue;
		}

		if (!strcmp(arg, "--exclude-promisor-objects"))
			continue; /* already handled above */
		if (skip_prefix(arg, "--missing=", &arg))
			continue; /* already handled above */

		if (!strcmp(arg, ("--no-object-names"))) {
			arg_show_object_names = 0;
			continue;
		}

		if (!strcmp(arg, ("--object-names"))) {
			arg_show_object_names = 1;
			continue;
		}

		if (!strcmp(arg, ("--commit-header"))) {
			revs.include_header = 1;
			continue;
		}

		if (!strcmp(arg, ("--no-commit-header"))) {
			revs.include_header = 0;
			continue;
		}

		if (!strcmp(arg, "--disk-usage")) {
			show_disk_usage = 1;
			info.flags |= REV_LIST_QUIET;
			continue;
		}

		usage(rev_list_usage);

	}
	if (revs.commit_format != CMIT_FMT_USERFORMAT)
		revs.include_header = 1;
	if (revs.commit_format != CMIT_FMT_UNSPECIFIED) {
		/* The command line has a --pretty  */
		info.hdr_termination = '\n';
		if (revs.commit_format == CMIT_FMT_ONELINE || !revs.include_header)
			info.header_prefix = "";
		else
			info.header_prefix = "commit ";
	}
	else if (revs.verbose_header)
		/* Only --header was specified */
		revs.commit_format = CMIT_FMT_RAW;

	if ((!revs.commits && reflog_walk_empty(revs.reflog_info) &&
	     (!(revs.tag_objects || revs.tree_objects || revs.blob_objects) &&
	      !revs.pending.nr) &&
	     !revs.rev_input_given && !revs.read_from_stdin) ||
	    revs.diff)
		usage(rev_list_usage);

	if (revs.show_notes)
		die(_("rev-list does not support display of notes"));

	if (revs.count &&
	    (revs.tag_objects || revs.tree_objects || revs.blob_objects) &&
	    (revs.left_right || revs.cherry_mark))
		die(_("marked counting is incompatible with --objects"));

	save_commit_buffer = (revs.verbose_header ||
			      revs.grep_filter.pattern_list ||
			      revs.grep_filter.header_list);
	if (bisect_list)
		revs.limited = 1;

	if (show_progress)
		progress = start_delayed_progress(show_progress, 0);

	if (use_bitmap_index) {
		if (!try_bitmap_count(&revs, &filter_options, filter_provided_objects))
			return 0;
		if (!try_bitmap_disk_usage(&revs, &filter_options, filter_provided_objects))
			return 0;
		if (!try_bitmap_traversal(&revs, &filter_options, filter_provided_objects))
			return 0;
	}

	if (prepare_revision_walk(&revs))
		die("revision walk setup failed");
	if (revs.tree_objects)
		mark_edges_uninteresting(&revs, show_edge, 0);

	if (bisect_list) {
		int reaches, all;
		unsigned bisect_flags = 0;

		if (bisect_find_all)
			bisect_flags |= FIND_BISECTION_ALL;

		if (revs.first_parent_only)
			bisect_flags |= FIND_BISECTION_FIRST_PARENT_ONLY;

		find_bisection(&revs.commits, &reaches, &all, bisect_flags);

		if (bisect_show_vars)
			return show_bisect_vars(&info, reaches, all);
	}


// Source: rev-list.c
// Lines 495-718
