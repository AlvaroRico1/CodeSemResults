static char *output_prefix(struct diff_options *opt)
{
	char *prefix = "";

	if (opt->output_prefix) {
		struct strbuf *sb = opt->output_prefix(opt, opt->output_prefix_data);
		prefix = sb->buf;
	}


// Source: line-log.c
// Lines 899-906
