static struct commit *lookup_label(const char *label, int len,
				   struct strbuf *buf)
{
	struct commit *commit;

	strbuf_reset(buf);
	strbuf_addf(buf, "refs/rewritten/%.*s", len, label);
	commit = lookup_commit_reference_by_name(buf->buf);
	if (!commit) {
		/* fall back to non-rewritten ref or commit */
		strbuf_splice(buf, 0, strlen("refs/rewritten/"), "", 0);
		commit = lookup_commit_reference_by_name(buf->buf);
	}


// Source: sequencer.c
// Lines 3737-3749
