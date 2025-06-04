options_remove(struct options_entry *o)
{
	struct options	*oo = o->owner;

	if (OPTIONS_IS_ARRAY(o))
		options_array_clear(o);
	else
		options_value_free(o, &o->value);
	RB_REMOVE(options_tree, &oo->tree, o);
	free((void *)o->name);
	free(o);
}


// Source: options.c
// Lines 337-348
