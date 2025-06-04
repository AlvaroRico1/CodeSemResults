format_table_compare(const void *key0, const void *entry0)
{
	const char			*key = key0;
	const struct format_table_entry	*entry = entry0;

	return (strcmp(key, entry->key));
}


// Source: format.c
// Lines 3010-3016
