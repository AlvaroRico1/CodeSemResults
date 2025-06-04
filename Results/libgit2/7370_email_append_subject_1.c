static int append_subject(
	git_str *out,
	size_t patch_idx,
	size_t patch_count,
	const char *summary,
	git_email_create_options *opts)
{
	bool prefix = include_prefix(patch_count, opts);
	size_t summary_len = summary ? strlen(summary) : 0;
	int error;

	if (summary_len) {
		const char *nl = strchr(summary, '\n');

		if (nl)
			summary_len = (nl - summary);
	}


// Source: email.c
// Lines 89-105
