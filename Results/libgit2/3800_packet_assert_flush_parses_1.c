static void assert_flush_parses(const char *line)
{
	size_t linelen = strlen(line) + 1;
	const char *endptr;
	git_pkt *pkt;

	cl_git_pass(git_pkt_parse_line((git_pkt **) &pkt, &endptr, line, linelen));
	cl_assert_equal_i(pkt->type, GIT_PKT_FLUSH);
	cl_assert_equal_strn(endptr, line + 4, linelen - 4);

	git_pkt_free((git_pkt *) pkt);
}


// Source: packet.c
// Lines 9-20
