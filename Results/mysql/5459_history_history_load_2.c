history_load(TYPE(History) *h, const char *fname)
{
	FILE *fp;
	char *line;
	size_t llen;
	ssize_t sz;
	size_t max_size;
	char *ptr;
	int i = -1;
	TYPE(HistEvent) ev;
	Char *decode_result;
#ifndef NARROWCHAR
	static ct_buffer_t conv;
#endif

	if ((fp = fopen(fname, "r")) == NULL)
		return i;

	line = NULL;
	llen = 0;
	if ((sz = getline(&line, &llen, fp)) == -1)
		goto done;

	if (strncmp(line, hist_cookie, (size_t)sz) != 0)
		goto done;

	ptr = h_malloc((max_size = 1024) * sizeof(*ptr));
	if (ptr == NULL)
		goto done;
	for (i = 0; (sz = getline(&line, &llen, fp)) != -1; i++) {
		if (sz > 0 && line[sz - 1] == '\n')
			line[--sz] = '\0';
		if (max_size < (size_t)sz) {
			char *nptr;
			max_size = ((size_t)sz + 1024) & (size_t)~1023;
			nptr = h_realloc(ptr, max_size * sizeof(*ptr));
			if (nptr == NULL) {
				i = -1;
				goto oomem;
			}
			ptr = nptr;
		}
		(void) strunvis(ptr, line);
		decode_result = ct_decode_string(ptr, &conv);
		if (decode_result == NULL)
			continue;
		if (HENTER(h, &ev, decode_result) == -1) {
			i = -1;
			goto oomem;
		}
	}
oomem:
	h_free(ptr);
done:
	free(line);
	(void) fclose(fp);
	return i;
}


// Source: history.c
// Lines 773-830
