int diff_line_cb(
	const git_diff_delta *delta,
	const git_diff_hunk *hunk,
	const git_diff_line *line,
	void *payload)
{
	diff_expects *e = payload;

	GIT_UNUSED(delta);
	GIT_UNUSED(hunk);

	e->lines++;
	switch (line->origin) {
	case GIT_DIFF_LINE_CONTEXT:
	case GIT_DIFF_LINE_CONTEXT_EOFNL: /* techically not a line */
		e->line_ctxt++;
		break;
	case GIT_DIFF_LINE_ADDITION:
	case GIT_DIFF_LINE_ADD_EOFNL: /* technically not a line add */
		e->line_adds++;
		break;
	case GIT_DIFF_LINE_DELETION:
	case GIT_DIFF_LINE_DEL_EOFNL: /* technically not a line delete */
		e->line_dels++;
		break;
	default:
		break;
	}
	return 0;
}


// Source: diff_helpers.c
// Lines 126-155
