static struct entry *rev_list_push(struct data *data, struct commit *commit, int mark)
{
	struct entry *entry;
	commit->object.flags |= mark | SEEN;

	CALLOC_ARRAY(entry, 1);
	entry->commit = commit;
	prio_queue_put(&data->rev_list, entry);

	if (!(mark & COMMON))
		data->non_common_revs++;
	return entry;
}


// Source: skipping.c
// Lines 60-72
