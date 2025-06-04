static int subject2item_cmp(const void *fndata,
			    const struct hashmap_entry *eptr,
			    const struct hashmap_entry *entry_or_key,
			    const void *key)
{
	const struct subject2item_entry *a, *b;

	a = container_of(eptr, const struct subject2item_entry, entry);
	b = container_of(entry_or_key, const struct subject2item_entry, entry);

	return key ? strcmp(a->subject, key) : strcmp(a->subject, b->subject);
}


// Source: sequencer.c
// Lines 5741-5752
