static void output_header_lines(FILE *fout, const char *hdr, const struct strbuf *data)
{
	const char *sp = data->buf;
	while (1) {
		char *ep = strchr(sp, '\n');
		int len;
		if (!ep)
			len = strlen(sp);
		else
			len = ep - sp;
		fprintf(fout, "%s: %.*s\n", hdr, len, sp);
		if (!ep)
			break;
		sp = ep + 1;
	}
}


// Source: mailinfo.c
// Lines 1130-1145
