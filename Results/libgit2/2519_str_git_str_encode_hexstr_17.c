int git_str_encode_hexstr(git_str *str, const char *data, size_t len)
{
	size_t new_size, i;
	char *s;

	GIT_ERROR_CHECK_ALLOC_MULTIPLY(&new_size, len, 2);
	GIT_ERROR_CHECK_ALLOC_ADD(&new_size, new_size, 1);

	if (git_str_grow_by(str, new_size) < 0)
		return -1;

	s = str->ptr + str->size;

	for (i = 0; i < len; i++) {
		*s++ = hex_encode[(data[i] & 0xf0) >> 4];
		*s++ = hex_encode[(data[i] & 0x0f)];
	}

	str->size += (len * 2);
	str->ptr[str->size] = '\0';

	return 0;
}


// Source: str.c
// Lines 222-244
