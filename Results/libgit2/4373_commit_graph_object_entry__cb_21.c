static int object_entry__cb(const git_oid *id, void *data)
{
	struct object_entry_cb_state *state = (struct object_entry_cb_state *)data;
	git_commit *commit = NULL;
	struct packed_commit *packed_commit = NULL;
	size_t header_len;
	git_object_t header_type;
	int error = 0;

	error = git_odb_read_header(&header_len, &header_type, state->db, id);
	if (error < 0)
		return error;

	if (header_type != GIT_OBJECT_COMMIT)
		return 0;

	error = git_commit_lookup(&commit, state->repo, id);
	if (error < 0)
		return error;

	packed_commit = packed_commit_new(commit);
	git_commit_free(commit);
	GIT_ERROR_CHECK_ALLOC(packed_commit);

	error = git_vector_insert(state->commits, packed_commit);
	if (error < 0) {
		packed_commit_free(packed_commit);
		return error;
	}

	return 0;
}


// Source: commit_graph.c
// Lines 667-698
