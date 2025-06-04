tty_acs_reverse_cmp(const void *key, const void *value)
{
	const struct tty_acs_reverse_entry	*entry = value;
	const char				*test = key;

	return (strcmp(test, entry->string));
}


// Source: tty-acs.c
// Lines 195-201
