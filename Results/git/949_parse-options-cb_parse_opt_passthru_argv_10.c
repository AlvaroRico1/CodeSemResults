int parse_opt_passthru_argv(const struct option *opt, const char *arg, int unset)
{
	static struct strbuf sb = STRBUF_INIT;
	struct strvec *opt_value = opt->value;

	if (recreate_opt(&sb, opt, arg, unset) < 0)
		return -1;

	strvec_push(opt_value, sb.buf);

	return 0;
}


// Source: parse-options-cb.c
// Lines 284-295
