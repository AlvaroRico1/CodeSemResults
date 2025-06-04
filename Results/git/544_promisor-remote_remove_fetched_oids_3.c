static int remove_fetched_oids(struct repository *repo,
			       struct object_id **oids,
			       int oid_nr, int to_free)
{
	int i, remaining_nr = 0;
	int *remaining = xcalloc(oid_nr, sizeof(*remaining));
	struct object_id *old_oids = *oids;
	struct object_id *new_oids;

	for (i = 0; i < oid_nr; i++)
		if (oid_object_info_extended(repo, &old_oids[i], NULL,
					     OBJECT_INFO_SKIP_FETCH_OBJECT)) {
			remaining[i] = 1;
			remaining_nr++;
		}

	if (remaining_nr) {
		int j = 0;
		CALLOC_ARRAY(new_oids, remaining_nr);
		for (i = 0; i < oid_nr; i++)
			if (remaining[i])
				oidcpy(&new_oids[j++], &old_oids[i]);
		*oids = new_oids;
		if (to_free)
			free(old_oids);
	}

	free(remaining);

	return remaining_nr;
}


// Source: promisor-remote.c
// Lines 201-231
