static const struct object_id *get_rev(struct data *data)
{
	struct commit *to_send = NULL;

	while (to_send == NULL) {
		struct entry *entry;
		struct commit *commit;
		struct commit_list *p;
		int parent_pushed = 0;

		if (data->rev_list.nr == 0 || data->non_common_revs == 0)
			return NULL;

		entry = prio_queue_get(&data->rev_list);
		commit = entry->commit;
		commit->object.flags |= POPPED;
		if (!(commit->object.flags & COMMON))
			data->non_common_revs--;

		if (!(commit->object.flags & COMMON) && !entry->ttl)
			to_send = commit;

		parse_commit(commit);
		for (p = commit->parents; p; p = p->next)
			parent_pushed |= push_parent(data, entry, p->item);

		if (!(commit->object.flags & COMMON) && !parent_pushed)
			/*
			 * This commit has no parents, or all of its parents
			 * have already been popped (due to clock skew), so send
			 * it anyway.
			 */
			to_send = commit;

		free(entry);
	}


// Source: skipping.c
// Lines 158-193
