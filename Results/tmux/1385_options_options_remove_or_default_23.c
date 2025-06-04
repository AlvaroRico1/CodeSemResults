options_remove_or_default(struct options_entry *o, int idx, char **cause)
{
	struct options	*oo = o->owner;

	if (idx == -1) {
		if (o->tableentry != NULL &&
		    (oo == global_options ||
		    oo == global_s_options ||
		    oo == global_w_options))
			options_default(oo, o->tableentry);
		else
			options_remove(o);
	} else if (options_array_set(o, idx, NULL, 0, cause) != 0)
		return (-1);
	return (0);
}


// Source: options.c
// Lines 1157-1172
