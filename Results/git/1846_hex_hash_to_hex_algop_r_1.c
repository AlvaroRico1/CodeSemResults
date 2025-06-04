char *hash_to_hex_algop_r(char *buffer, const unsigned char *hash,
			  const struct git_hash_algo *algop)
{
	static const char hex[] = "0123456789abcdef";
	char *buf = buffer;
	int i;

	/*
	 * Our struct object_id has been memset to 0, so default to printing
	 * using the default hash.
	 */
	if (algop == &hash_algos[0])
		algop = the_hash_algo;

	for (i = 0; i < algop->rawsz; i++) {
		unsigned int val = *hash++;
		*buf++ = hex[val >> 4];
		*buf++ = hex[val & 0xf];
	}
	*buf = '\0';

	return buffer;
}


// Source: hex.c
// Lines 120-142
