static int clar_summary_close_tag(
    struct clar_summary *summary, const char *tag, int indent)
{
	const char *indt;

	if (indent == 0) indt = "";
	else if (indent == 1) indt = "\t";
	else indt = "\t\t";

	return fprintf(summary->fp, "%s</%s>\n", indt, tag);
}


// Source: summary.h
// Lines 5-15
