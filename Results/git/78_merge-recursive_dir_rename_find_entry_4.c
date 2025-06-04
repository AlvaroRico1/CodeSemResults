static struct dir_rename_entry *dir_rename_find_entry(struct hashmap *hashmap,
						      char *dir)
{
	struct dir_rename_entry key;

	if (dir == NULL)
		return NULL;
	hashmap_entry_init(&key.ent, strhash(dir));
	key.dir = dir;
	return hashmap_get_entry(hashmap, &key, ent, NULL);
}


// Source: merge-recursive.c
// Lines 80-90
