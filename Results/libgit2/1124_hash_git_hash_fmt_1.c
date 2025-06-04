int git_hash_fmt(char *out, unsigned char *hash, size_t hash_len)
{
	static char hex[] = "0123456789abcdef";
	char *str = out;
	size_t i;

	for (i = 0; i < hash_len; i++) {
		*str++ = hex[hash[i] >> 4];
		*str++ = hex[hash[i] & 0x0f];
	}

	*str++ = '\0';

	return 0;
}


// Source: hash.c
// Lines 144-158
