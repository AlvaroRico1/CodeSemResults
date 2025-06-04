popup_editor_close_cb(int status, void *arg)
{
	struct popup_editor	*pe = arg;
	FILE			*f;
	char			*buf = NULL;
	off_t			 len = 0;

	if (status != 0) {
		pe->cb(NULL, 0, pe->arg);
		popup_editor_free(pe);
		return;
	}

	f = fopen(pe->path, "r");
	if (f != NULL) {
		fseeko(f, 0, SEEK_END);
		len = ftello(f);
		fseeko(f, 0, SEEK_SET);

		if (len == 0 ||
		    (uintmax_t)len > (uintmax_t)SIZE_MAX ||
		    (buf = malloc(len)) == NULL ||
		    fread(buf, len, 1, f) != 1) {
			free(buf);
			buf = NULL;
			len = 0;
		}
		fclose(f);
	}
	pe->cb(buf, len, pe->arg); /* callback now owns buffer */
	popup_editor_free(pe);
}


// Source: popup.c
// Lines 711-742
