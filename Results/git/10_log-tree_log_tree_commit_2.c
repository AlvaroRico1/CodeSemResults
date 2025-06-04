int log_tree_commit(struct rev_info *opt, struct commit *commit)
{
	struct log_info log;
	int shown;
	/* maybe called by e.g. cmd_log_walk(), maybe stand-alone */
	int no_free = opt->diffopt.no_free;

	log.commit = commit;
	log.parent = NULL;
	opt->loginfo = &log;
	opt->diffopt.no_free = 1;

	if (opt->line_level_traverse)
		return line_log_print(opt, commit);

	if (opt->track_linear && !opt->linear && !opt->reverse_output_stage)
		fprintf(opt->diffopt.file, "\n%s\n", opt->break_bar);
	shown = log_tree_diff(opt, commit, &log);
	if (!shown && opt->loginfo && opt->always_show_header) {
		log.parent = NULL;
		show_log(opt);
		shown = 1;
	}
	if (opt->track_linear && !opt->linear && opt->reverse_output_stage)
		fprintf(opt->diffopt.file, "\n%s\n", opt->break_bar);
	opt->loginfo = NULL;
	maybe_flush_or_die(opt->diffopt.file, "stdout");
	opt->diffopt.no_free = no_free;
	diff_free(&opt->diffopt);
	return shown;
}


// Source: log-tree.c
// Lines 971-1001
