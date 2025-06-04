tty_acs_cmp(const void *key, const void *value)
{
	const struct tty_acs_entry	*entry = value;
	int				 test = *(u_char *)key;

	return (test - entry->key);
}


// Source: tty-acs.c
// Lines 186-192
