static long diff_context_find(
	const char *line,
	long line_len,
	char *out,
	long out_size,
	void *payload)
{
	git_diff_find_context_payload *ctxt = payload;

	if (git_str_set(&ctxt->line, line, (size_t)line_len) < 0)
		return -1;
	git_str_rtrim(&ctxt->line);

	if (!ctxt->line.size)
		return -1;

	if (!ctxt->match_line || !ctxt->match_line(ctxt->driver, &ctxt->line))
		return -1;

	if (out_size > (long)ctxt->line.size)
		out_size = (long)ctxt->line.size;
	memcpy(out, ctxt->line.ptr, (size_t)out_size);

	return out_size;
}


// Source: diff_driver.c
// Lines 474-498
