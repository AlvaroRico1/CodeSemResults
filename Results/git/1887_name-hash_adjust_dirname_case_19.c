void adjust_dirname_case(struct index_state *istate, char *name)
{
	const char *startPtr = name;
	const char *ptr = startPtr;

	lazy_init_name_hash(istate);
	expand_to_path(istate, name, strlen(name), 0);
	while (*ptr) {
		while (*ptr && *ptr != '/')
			ptr++;

		if (*ptr == '/') {
			struct dir_entry *dir;

			dir = find_dir_entry(istate, name, ptr - name);
			if (dir) {
				memcpy((void *)startPtr, dir->name + (startPtr - name), ptr - startPtr);
				startPtr = ptr + 1;
			}
			ptr++;
		}
	}
}


// Source: name-hash.c
// Lines 692-714
