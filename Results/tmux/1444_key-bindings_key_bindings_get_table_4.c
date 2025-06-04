key_bindings_get_table(const char *name, int create)
{
	struct key_table	table_find, *table;

	table_find.name = name;
	table = RB_FIND(key_tables, &key_tables, &table_find);
	if (table != NULL || !create)
		return (table);

	table = xmalloc(sizeof *table);
	table->name = xstrdup(name);
	RB_INIT(&table->key_bindings);
	RB_INIT(&table->default_key_bindings);

	table->references = 1; /* one reference in key_tables */
	RB_INSERT(key_tables, &key_tables, table);

	return (table);
}


// Source: key-bindings.c
// Lines 100-118
