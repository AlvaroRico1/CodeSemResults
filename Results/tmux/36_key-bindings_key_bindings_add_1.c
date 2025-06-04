key_bindings_add(const char *name, key_code key, const char *note, int repeat,
    struct cmd_list *cmdlist)
{
	struct key_table	*table;
	struct key_binding	*bd;
	char			*s;

	table = key_bindings_get_table(name, 1);

	bd = key_bindings_get(table, key & ~KEYC_MASK_FLAGS);
	if (cmdlist == NULL) {
		if (bd != NULL) {
			free((void *)bd->note);
			if (note != NULL)
				bd->note = xstrdup(note);
			else
				bd->note = NULL;
		}
		return;
	}
	if (bd != NULL) {
		RB_REMOVE(key_bindings, &table->key_bindings, bd);
		key_bindings_free(bd);
	}

	bd = xcalloc(1, sizeof *bd);
	bd->key = (key & ~KEYC_MASK_FLAGS);
	if (note != NULL)
		bd->note = xstrdup(note);
	RB_INSERT(key_bindings, &table->key_bindings, bd);

	if (repeat)
		bd->flags |= KEY_BINDING_REPEAT;
	bd->cmdlist = cmdlist;

	s = cmd_list_print(bd->cmdlist, 0);
	log_debug("%s: %#llx %s = %s", __func__, bd->key,
	    key_string_lookup_key(bd->key, 1), s);
	free(s);
}


// Source: key-bindings.c
// Lines 185-224
