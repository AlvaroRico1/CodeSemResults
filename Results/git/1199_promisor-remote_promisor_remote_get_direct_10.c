int promisor_remote_get_direct(struct repository *repo,
			       const struct object_id *oids,
			       int oid_nr)
{
	struct promisor_remote *r;
	struct object_id *remaining_oids = (struct object_id *)oids;
	int remaining_nr = oid_nr;
	int to_free = 0;
	int res = -1;

	if (oid_nr == 0)
		return 0;

	promisor_remote_init(repo);

	for (r = repo->promisor_remote_config->promisors; r; r = r->next) {
		if (fetch_objects(repo, r->name, remaining_oids, remaining_nr) < 0) {
			if (remaining_nr == 1)
				continue;
			remaining_nr = remove_fetched_oids(repo, &remaining_oids,
							 remaining_nr, to_free);
			if (remaining_nr) {
				to_free = 1;
				continue;
			}
		}
		res = 0;
		break;
	}

	if (to_free)
		free(remaining_oids);

	return res;
}


// Source: promisor-remote.c
// Lines 233-267
