fat_short2lower_uni(struct nls_table *t, unsigned char *c,
		    int clen, wchar_t *uni)
{
	int charlen;
	wchar_t wc;

	charlen = t->char2uni(c, clen, &wc);
	if (charlen < 0) {
		*uni = 0x003f;	/* a question mark */
		charlen = 1;
	} else if (charlen <= 1) {
		unsigned char nc = t->charset2lower[*c];

		if (!nc)
			nc = *c;

		charlen = t->char2uni(&nc, 1, uni);
		if (charlen < 0) {
			*uni = 0x003f;	/* a question mark */
			charlen = 1;
		}
	} else


// Source: dir.c
// Lines 208-229
