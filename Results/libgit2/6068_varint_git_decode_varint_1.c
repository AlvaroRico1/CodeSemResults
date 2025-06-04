uintmax_t git_decode_varint(const unsigned char *bufp, size_t *varint_len)
{
	const unsigned char *buf = bufp;
	unsigned char c = *buf++;
	uintmax_t val = c & 127;
	while (c & 128) {
		val += 1;
		if (!val || MSB(val, 7)) {
			/* This is not a valid varint_len, so it signals
			   the error */
			*varint_len = 0;
			return 0; /* overflow */
		}
		c = *buf++;
		val = (val << 7) + (c & 127);
	}
	*varint_len = buf - bufp;
	return val;
}


// Source: varint.c
// Lines 10-28
