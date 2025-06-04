static int cmd_reflog_expire(int argc, const char **argv, const char *prefix)
{
	struct expire_reflog_policy_cb cb;
	timestamp_t now = time(NULL);
	int i, status, do_all, all_worktrees = 1;
	int explicit_expiry = 0;
	unsigned int flags = 0;

	default_reflog_expire_unreachable = now - 30 * 24 * 3600;
	default_reflog_expire = now - 90 * 24 * 3600;
	git_config(reflog_expire_config, NULL);

	save_commit_buffer = 0;
	do_all = status = 0;
	memset(&cb, 0, sizeof(cb));

	cb.cmd.expire_total = default_reflog_expire;
	cb.cmd.expire_unreachable = default_reflog_expire_unreachable;

	for (i = 1; i < argc; i++) {
		const char *arg = argv[i];

		if (!strcmp(arg, "--dry-run") || !strcmp(arg, "-n"))
			flags |= EXPIRE_REFLOGS_DRY_RUN;
		else if (skip_prefix(arg, "--expire=", &arg)) {
			if (parse_expiry_date(arg, &cb.cmd.expire_total))
				die(_("'%s' is not a valid timestamp"), arg);
			explicit_expiry |= EXPIRE_TOTAL;
		}
		else if (skip_prefix(arg, "--expire-unreachable=", &arg)) {
			if (parse_expiry_date(arg, &cb.cmd.expire_unreachable))
				die(_("'%s' is not a valid timestamp"), arg);
			explicit_expiry |= EXPIRE_UNREACH;
		}
		else if (!strcmp(arg, "--stale-fix"))
			cb.cmd.stalefix = 1;
		else if (!strcmp(arg, "--rewrite"))
			flags |= EXPIRE_REFLOGS_REWRITE;
		else if (!strcmp(arg, "--updateref"))
			flags |= EXPIRE_REFLOGS_UPDATE_REF;
		else if (!strcmp(arg, "--all"))
			do_all = 1;
		else if (!strcmp(arg, "--single-worktree"))
			all_worktrees = 0;
		else if (!strcmp(arg, "--verbose"))
			flags |= EXPIRE_REFLOGS_VERBOSE;
		else if (!strcmp(arg, "--")) {
			i++;
			break;
		}
		else if (arg[0] == '-')
			usage(_(reflog_expire_usage));
		else
			break;
	}

	/*
	 * We can trust the commits and objects reachable from refs
	 * even in older repository.  We cannot trust what's reachable
	 * from reflog if the repository was pruned with older git.
	 */
	if (cb.cmd.stalefix) {
		repo_init_revisions(the_repository, &cb.cmd.revs, prefix);
		cb.cmd.revs.do_not_die_on_missing_tree = 1;
		cb.cmd.revs.ignore_missing = 1;
		cb.cmd.revs.ignore_missing_links = 1;
		if (flags & EXPIRE_REFLOGS_VERBOSE)
			printf(_("Marking reachable objects..."));
		mark_reachable_objects(&cb.cmd.revs, 0, 0, NULL);
		if (flags & EXPIRE_REFLOGS_VERBOSE)
			putchar('\n');
	}

	if (do_all) {
		struct collect_reflog_cb collected;
		struct worktree **worktrees, **p;
		int i;

		memset(&collected, 0, sizeof(collected));
		worktrees = get_worktrees();
		for (p = worktrees; *p; p++) {
			if (!all_worktrees && !(*p)->is_current)
				continue;
			collected.wt = *p;
			refs_for_each_reflog(get_worktree_ref_store(*p),
					     collect_reflog, &collected);
		}
		free_worktrees(worktrees);
		for (i = 0; i < collected.nr; i++) {
			struct collected_reflog *e = collected.e[i];

			set_reflog_expiry_param(&cb.cmd, explicit_expiry, e->reflog);
			status |= reflog_expire(e->reflog, flags,
						reflog_expiry_prepare,
						should_expire_reflog_ent,
						reflog_expiry_cleanup,
						&cb);
			free(e);
		}
		free(collected.e);
	}


// Source: reflog.c
// Lines 542-642
