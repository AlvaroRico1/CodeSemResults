static struct collision_entry *collision_find_entry(struct hashmap *hashmap,
						    char *target_file)
{
	struct collision_entry key;

	hashmap_entry_init(&key.ent, strhash(target_file));
	key.target_file = target_file;
	return hashmap_get_entry(hashmap, &key, ent, NULL);
}


// Source: merge-recursive.c
// Lines 127-135
