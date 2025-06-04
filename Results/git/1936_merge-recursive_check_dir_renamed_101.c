static struct dir_rename_entry *check_dir_renamed(const char *path,
						  struct hashmap *dir_renames)
{
	char *temp = xstrdup(path);
	char *end;
	struct dir_rename_entry *entry = NULL;

	while ((end = strrchr(temp, '/'))) {
		*end = '\0';
		entry = dir_rename_find_entry(dir_renames, temp);
		if (entry)
			break;
	}
	free(temp);
	return entry;
}


// Source: merge-recursive.c
// Lines 2360-2375
