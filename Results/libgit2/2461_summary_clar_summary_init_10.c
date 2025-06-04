struct clar_summary *clar_summary_init(const char *filename)
{
	struct clar_summary *summary;
	FILE *fp;

	if ((fp = fopen(filename, "w")) == NULL)
		return NULL;

	if ((summary = malloc(sizeof(struct clar_summary))) == NULL) {
		fclose(fp);
		return NULL;
	}

	summary->filename = filename;
	summary->fp = fp;

	return summary;
}


// Source: summary.h
// Lines 61-78
