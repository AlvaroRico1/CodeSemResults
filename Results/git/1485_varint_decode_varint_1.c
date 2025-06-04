uintmax_t decode_varint(const unsigned char **bufp)
{
	const unsigned char *buf = *bufp;
	unsigned char c = *buf++;
	uintmax_t val = c & 127;
	while (c & 128) {
		val += 1;
		if (!val || MSB(val, 7))
			return 0; /* overflow */
		c = *buf++;
		val = (val << 7) + (c & 127);
	}
	*bufp = buf;
	return val;
}


// Source: varint.c
// Lines 4-18
