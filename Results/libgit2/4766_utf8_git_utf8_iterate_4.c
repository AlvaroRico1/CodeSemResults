int git_utf8_iterate(uint32_t *out, const char *_str, size_t str_len)
{
	const uint8_t *str = (const uint8_t *)_str;
	uint32_t uc = 0;
	int length;

	*out = 0;

	if ((length = utf8_charlen(str, str_len)) < 0)
		return -1;

	switch (length) {
		case 1:
			uc = str[0];
			break;
		case 2:
			uc = ((str[0] & 0x1F) <<  6) + (str[1] & 0x3F);
			if (uc < 0x80) uc = -1;
			break;
		case 3:
			uc = ((str[0] & 0x0F) << 12) + ((str[1] & 0x3F) <<  6)
				+ (str[2] & 0x3F);
			if (uc < 0x800 || (uc >= 0xD800 && uc < 0xE000) ||
					(uc >= 0xFDD0 && uc < 0xFDF0)) uc = -1;
			break;
		case 4:
			uc = ((str[0] & 0x07) << 18) + ((str[1] & 0x3F) << 12)
				+ ((str[2] & 0x3F) <<  6) + (str[3] & 0x3F);
			if (uc < 0x10000 || uc >= 0x110000) uc = -1;
			break;
		default:
			return -1;
	}

	if ((uc & 0xFFFF) >= 0xFFFE)
		return -1;

	*out = uc;
	return length;
}


// Source: utf8.c
// Lines 76-115
