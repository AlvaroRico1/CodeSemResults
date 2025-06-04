void *oidmap_put(struct oidmap *map, void *entry)
{
	struct oidmap_entry *to_put = entry;

	if (!map->map.cmpfn)
		oidmap_init(map, 0);

	hashmap_entry_init(&to_put->internal_entry, oidhash(&to_put->oid));
	return hashmap_put(&map->map, &to_put->internal_entry);
}


// Source: oidmap.c
// Lines 52-61
