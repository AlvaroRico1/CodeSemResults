static int notes_display_config(const char *k, const char *v, void *cb)
{
	int *load_refs = cb;

	if (*load_refs && !strcmp(k, "notes.displayref")) {
		if (!v)
			return config_error_nonbool(k);
		string_list_add_refs_by_glob(&display_notes_refs, v);
	}

	return 0;
}


// Source: notes.c
// Lines 970-981
