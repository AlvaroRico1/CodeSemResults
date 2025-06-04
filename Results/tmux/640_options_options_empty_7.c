options_empty(struct options *oo, const struct options_table_entry *oe)
{
	struct options_entry	*o;

	o = options_add(oo, oe->name);
	o->tableentry = oe;

	if (oe->flags & OPTIONS_TABLE_IS_ARRAY)
		RB_INIT(&o->value.array);

	return (o);
}


// Source: options.c
// Lines 244-255
