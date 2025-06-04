enum bisect_error bisect_next_all(struct repository *r, const char *prefix)
{
	struct rev_info revs;
	struct commit_list *tried;
	int reaches = 0, all = 0, nr, steps;
	enum bisect_error res = BISECT_OK;
	struct object_id *bisect_rev;
	char *steps_msg;
	/*
	 * If no_checkout is non-zero, the bisection process does not
	 * checkout the trial commit but instead simply updates BISECT_HEAD.
	 */
	int no_checkout = ref_exists("BISECT_HEAD");
	unsigned bisect_flags = 0;

	read_bisect_terms(&term_bad, &term_good);
	if (read_bisect_refs())
		die(_("reading bisect refs failed"));

	if (file_exists(git_path_bisect_first_parent()))
		bisect_flags |= FIND_BISECTION_FIRST_PARENT_ONLY;

	if (skipped_revs.nr)
		bisect_flags |= FIND_BISECTION_ALL;

	res = check_good_are_ancestors_of_bad(r, prefix, no_checkout);
	if (res)
		return res;

	bisect_rev_setup(r, &revs, prefix, "%s", "^%s", 1);

	revs.first_parent_only = !!(bisect_flags & FIND_BISECTION_FIRST_PARENT_ONLY);
	revs.limited = 1;

	bisect_common(&revs);

	find_bisection(&revs.commits, &reaches, &all, bisect_flags);
	revs.commits = managed_skipped(revs.commits, &tried);

	if (!revs.commits) {
		/*
		 * We should return error here only if the "bad"
		 * commit is also a "skip" commit.
		 */
		res = error_if_skipped_commits(tried, NULL);
		if (res < 0)
			return res;
		printf(_("%s was both %s and %s\n"),
		       oid_to_hex(current_bad_oid),
		       term_good,
		       term_bad);

		return BISECT_FAILED;
	}

	if (!all) {
		fprintf(stderr, _("No testable commit found.\n"
			"Maybe you started with bad path arguments?\n"));

		return BISECT_NO_TESTABLE_COMMIT;
	}

	bisect_rev = &revs.commits->item->object.oid;

	if (oideq(bisect_rev, current_bad_oid)) {
		res = error_if_skipped_commits(tried, current_bad_oid);
		if (res)
			return res;
		printf("%s is the first %s commit\n", oid_to_hex(bisect_rev),
			term_bad);

		show_diff_tree(r, prefix, revs.commits->item);
		/*
		 * This means the bisection process succeeded.
		 * Using BISECT_INTERNAL_SUCCESS_1ST_BAD_FOUND (-10)
		 * so that the call chain can simply check
		 * for negative return values for early returns up
		 * until the cmd_bisect__helper() caller.
		 */
		return BISECT_INTERNAL_SUCCESS_1ST_BAD_FOUND;
	}

	nr = all - reaches - 1;
	steps = estimate_bisect_steps(all);

	steps_msg = xstrfmt(Q_("(roughly %d step)", "(roughly %d steps)",
		  steps), steps);
	/*
	 * TRANSLATORS: the last %s will be replaced with "(roughly %d
	 * steps)" translation.
	 */
	printf(Q_("Bisecting: %d revision left to test after this %s\n",
		  "Bisecting: %d revisions left to test after this %s\n",
		  nr), nr, steps_msg);
	free(steps_msg);
	/* Clean up objects used, as they will be reused. */
	repo_clear_commit_marks(r, ALL_REV_FLAGS);

	return bisect_checkout(bisect_rev, no_checkout);
}


// Source: bisect.c
// Lines 1008-1107
