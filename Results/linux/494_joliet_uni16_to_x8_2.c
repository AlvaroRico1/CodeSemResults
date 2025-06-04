uni16_to_x8(unsigned char *ascii, __be16 *uni, int len, struct nls_table *nls)
{
	__be16 *ip, ch;
	unsigned char *op;

	ip = uni;
	op = ascii;

	while ((ch = get_unaligned(ip)) && len) {
		int llen;
		llen = nls->uni2char(be16_to_cpu(ch), op, NLS_MAX_CHARSET_SIZE);
		if (llen > 0)
			op += llen;
		else
			*op++ = '?';
		ip++;

		len--;
	}
	*op = 0;
	return (op - ascii);
}


// Source: joliet.c
// Lines 18-39
