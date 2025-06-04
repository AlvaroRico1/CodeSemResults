int parse_opt_passthru(const struct option *opt, const char *arg, int unset)
{
	static struct strbuf sb = STRBUF_INIT;
	char **opt_value = opt->value;

	if (recreate_opt(&sb, opt, arg, unset) < 0)
		return -1;

	free(*opt_value);

	*opt_value = strbuf_detach(&sb, NULL);

	return 0;
}


// Source: parse-options-cb.c
// Lines 263-276
