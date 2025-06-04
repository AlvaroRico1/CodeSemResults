size_t git_utf8_char_length(const char *_str, size_t str_len)
{
	const uint8_t *str = (const uint8_t *)_str;
	size_t offset = 0, count = 0;

	while (offset < str_len) {
		int length = utf8_charlen(str + offset, str_len - offset);

		if (length < 0)
			length = 1;

		offset += length;
		count++;
	}

	return count;
}


// Source: utf8.c
// Lines 117-133
