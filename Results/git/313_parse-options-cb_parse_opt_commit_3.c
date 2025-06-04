int parse_opt_commit(const struct option *opt, const char *arg, int unset)
{
	struct object_id oid;
	struct commit *commit;
	struct commit **target = opt->value;

	BUG_ON_OPT_NEG(unset);

	if (!arg)
		return -1;
	if (get_oid(arg, &oid))
		return error("malformed object name %s", arg);
	commit = lookup_commit_reference(the_repository, &oid);
	if (!commit)
		return error("no such commit %s", arg);
	*target = commit;
	return 0;
}


// Source: parse-options-cb.c
// Lines 102-119
