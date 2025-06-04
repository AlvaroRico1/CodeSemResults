static int local_download_pack(
		git_transport *transport,
		git_repository *repo,
		git_indexer_progress *stats)
{
	transport_local *t = (transport_local*)transport;
	git_revwalk *walk = NULL;
	git_remote_head *rhead;
	unsigned int i;
	int error = -1;
	git_packbuilder *pack = NULL;
	git_odb_writepack *writepack = NULL;
	git_odb *odb = NULL;
	git_str progress_info = GIT_STR_INIT;
	foreach_data data = {0};

	if ((error = git_revwalk_new(&walk, t->repo)) < 0)
		goto cleanup;

	git_revwalk_sorting(walk, GIT_SORT_TIME);

	if ((error = git_packbuilder_new(&pack, t->repo)) < 0)
		goto cleanup;

	git_packbuilder_set_callbacks(pack, local_counting, t);

	stats->total_objects = 0;
	stats->indexed_objects = 0;
	stats->received_objects = 0;
	stats->received_bytes = 0;

	git_vector_foreach(&t->refs, i, rhead) {
		git_object *obj;
		if ((error = git_object_lookup(&obj, t->repo, &rhead->oid, GIT_OBJECT_ANY)) < 0)
			goto cleanup;

		if (git_object_type(obj) == GIT_OBJECT_COMMIT) {
			/* Revwalker includes only wanted commits */
			error = git_revwalk_push(walk, &rhead->oid);
		} else {
			/* Tag or some other wanted object. Add it on its own */
			error = git_packbuilder_insert_recur(pack, &rhead->oid, rhead->name);
		}
		git_object_free(obj);
		if (error < 0)
			goto cleanup;
	}


// Source: local.c
// Lines 556-602
