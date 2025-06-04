static int dir_entry_cmp(const void *unused_cmp_data,
			 const struct hashmap_entry *eptr,
			 const struct hashmap_entry *entry_or_key,
			 const void *keydata)
{
	const struct dir_entry *e1, *e2;
	const char *name = keydata;

	e1 = container_of(eptr, const struct dir_entry, ent);
	e2 = container_of(entry_or_key, const struct dir_entry, ent);

	return e1->namelen != e2->namelen || strncasecmp(e1->name,
			name ? name : e2->name, e1->namelen);
}


// Source: name-hash.c
// Lines 21-34
