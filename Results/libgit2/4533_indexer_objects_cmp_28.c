static int objects_cmp(const void *a, const void *b)
{
	const struct entry *entrya = a;
	const struct entry *entryb = b;

	return git_oid__cmp(&entrya->oid, &entryb->oid);
}


// Source: indexer.c
// Lines 117-123
