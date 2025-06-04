int parse_opt_tertiary(const struct option *opt, const char *arg, int unset)
{
	int *target = opt->value;

	BUG_ON_OPT_ARG(arg);

	*target = unset ? 2 : 1;
	return 0;
}


// Source: parse-options-cb.c
// Lines 154-162
