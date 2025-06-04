static int format_binary(
	diff_print_info *pi,
	git_diff_binary_t type,
	const char *data,
	size_t datalen,
	size_t inflatedlen)
{
	const char *typename = type == GIT_DIFF_BINARY_DELTA ?
		"delta" : "literal";
	const char *scan, *end;

	git_str_printf(pi->buf, "%s %" PRIuZ "\n", typename, inflatedlen);
	pi->line.num_lines++;

	for (scan = data, end = data + datalen; scan < end; ) {
		size_t chunk_len = end - scan;
		if (chunk_len > 52)
			chunk_len = 52;

		if (chunk_len <= 26)
			git_str_putc(pi->buf, (char)chunk_len + 'A' - 1);
		else
			git_str_putc(pi->buf, (char)chunk_len - 26 + 'a' - 1);

		git_str_encode_base85(pi->buf, scan, chunk_len);
		git_str_putc(pi->buf, '\n');

		if (git_str_oom(pi->buf))
			return -1;

		scan += chunk_len;
		pi->line.num_lines++;
	}
	git_str_putc(pi->buf, '\n');

	if (git_str_oom(pi->buf))
		return -1;

	return 0;
}


// Source: diff_print.c
// Lines 460-499
