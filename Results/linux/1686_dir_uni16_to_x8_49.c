static int uni16_to_x8(struct super_block *sb, unsigned char *ascii,
		       const wchar_t *uni, int len, struct nls_table *nls)
{
	int uni_xlate = MSDOS_SB(sb)->options.unicode_xlate;
	const wchar_t *ip;
	wchar_t ec;
	unsigned char *op;
	int charlen;

	ip = uni;
	op = ascii;

	while (*ip && ((len - NLS_MAX_CHARSET_SIZE) > 0)) {
		ec = *ip++;
		charlen = nls->uni2char(ec, op, NLS_MAX_CHARSET_SIZE);
		if (charlen > 0) {
			op += charlen;
			len -= charlen;
		} else {
			if (uni_xlate == 1) {
				*op++ = ':';
				op = hex_byte_pack(op, ec >> 8);
				op = hex_byte_pack(op, ec);
				len -= 5;
			} else {
				*op++ = '?';
				len--;
			}
		}
	}

	if (unlikely(*ip)) {
		fat_msg(sb, KERN_WARNING,
			"filename was truncated while converting.");
	}

	*op = 0;
	return op - ascii;
}


// Source: dir.c
// Lines 143-181
