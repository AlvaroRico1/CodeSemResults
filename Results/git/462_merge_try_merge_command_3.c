int try_merge_command(struct repository *r,
		      const char *strategy, size_t xopts_nr,
		      const char **xopts, struct commit_list *common,
		      const char *head_arg, struct commit_list *remotes)
{
	struct strvec args = STRVEC_INIT;
	int i, ret;
	struct commit_list *j;

	strvec_pushf(&args, "merge-%s", strategy);
	for (i = 0; i < xopts_nr; i++)
		strvec_pushf(&args, "--%s", xopts[i]);
	for (j = common; j; j = j->next)
		strvec_push(&args, merge_argument(j->item));
	strvec_push(&args, "--");
	strvec_push(&args, head_arg);
	for (j = remotes; j; j = j->next)
		strvec_push(&args, merge_argument(j->item));

	ret = run_command_v_opt(args.v, RUN_GIT_CMD);
	strvec_clear(&args);

	discard_index(r->index);
	if (repo_read_index(r) < 0)
		die(_("failed to read the cache"));
	resolve_undo_clear_index(r->index);

	return ret;
}


// Source: merge.c
// Lines 17-45
