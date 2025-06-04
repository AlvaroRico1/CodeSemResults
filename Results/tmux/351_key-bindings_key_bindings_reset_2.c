key_bindings_reset(const char *name, key_code key)
{
	struct key_table	*table;
	struct key_binding	*bd, *dd;

	table = key_bindings_get_table(name, 0);
	if (table == NULL)
		return;

	bd = key_bindings_get(table, key & ~KEYC_MASK_FLAGS);
	if (bd == NULL)
		return;

	dd = key_bindings_get_default(table, bd->key);
	if (dd == NULL) {
		key_bindings_remove(name, bd->key);
		return;
	}

	cmd_list_free(bd->cmdlist);
	bd->cmdlist = dd->cmdlist;
	bd->cmdlist->references++;

	free((void *)bd->note);
	if (dd->note != NULL)
		bd->note = xstrdup(dd->note);
	else
		bd->note = NULL;
	bd->flags = dd->flags;
}


// Source: key-bindings.c
// Lines 254-283
