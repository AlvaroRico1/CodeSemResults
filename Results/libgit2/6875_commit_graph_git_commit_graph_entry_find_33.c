int git_commit_graph_entry_find(
		git_commit_graph_entry *e,
		const git_commit_graph_file *file,
		const git_oid *short_oid,
		size_t len)
{
	int pos, found = 0;
	uint32_t hi, lo;
	const git_oid *current = NULL;

	GIT_ASSERT_ARG(e);
	GIT_ASSERT_ARG(file);
	GIT_ASSERT_ARG(short_oid);

	hi = ntohl(file->oid_fanout[(int)short_oid->id[0]]);
	lo = ((short_oid->id[0] == 0x0) ? 0 : ntohl(file->oid_fanout[(int)short_oid->id[0] - 1]));

	pos = git_pack__lookup_sha1(file->oid_lookup, GIT_OID_RAWSZ, lo, hi, short_oid->id);

	if (pos >= 0) {
		/* An object matching exactly the oid was found */
		found = 1;
		current = file->oid_lookup + pos;
	} else {
		/* No object was found */
		/* pos refers to the object with the "closest" oid to short_oid */
		pos = -1 - pos;
		if (pos < (int)file->num_commits) {
			current = file->oid_lookup + pos;

			if (!git_oid_ncmp(short_oid, current, len))
				found = 1;
		}
	}

	if (found && len != GIT_OID_HEXSZ && pos + 1 < (int)file->num_commits) {
		/* Check for ambiguousity */
		const git_oid *next = current + 1;

		if (!git_oid_ncmp(short_oid, next, len)) {
			found = 2;
		}
	}


// Source: commit_graph.c
// Lines 509-551
