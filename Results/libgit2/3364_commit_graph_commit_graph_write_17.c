static int commit_graph_write(
		git_commit_graph_writer *w,
		commit_graph_write_cb write_cb,
		void *cb_data)
{
	int error = 0;
	size_t i;
	struct packed_commit *packed_commit;
	struct git_commit_graph_header hdr = {0};
	uint32_t oid_fanout_count;
	uint32_t extra_edge_list_count;
	uint32_t oid_fanout[256];
	off64_t offset;
	git_str oid_lookup = GIT_STR_INIT, commit_data = GIT_STR_INIT,
		extra_edge_list = GIT_STR_INIT;
	unsigned char checksum[GIT_HASH_SHA1_SIZE];
	size_t checksum_size;
	git_hash_ctx ctx;
	struct commit_graph_write_hash_context hash_cb_data = {0};

	hdr.signature = htonl(COMMIT_GRAPH_SIGNATURE);
	hdr.version = COMMIT_GRAPH_VERSION;
	hdr.object_id_version = COMMIT_GRAPH_OBJECT_ID_VERSION;
	hdr.chunks = 0;
	hdr.base_graph_files = 0;
	hash_cb_data.write_cb = write_cb;
	hash_cb_data.cb_data = cb_data;
	hash_cb_data.ctx = &ctx;

	checksum_size = GIT_HASH_SHA1_SIZE;
	error = git_hash_ctx_init(&ctx, GIT_HASH_ALGORITHM_SHA1);
	if (error < 0)
		return error;
	cb_data = &hash_cb_data;
	write_cb = commit_graph_write_hash;

	/* Sort the commits. */
	git_vector_sort(&w->commits);
	git_vector_uniq(&w->commits, packed_commit_free_dup);
	error = compute_generation_numbers(&w->commits);
	if (error < 0)
		goto cleanup;

	/* Fill the OID Fanout table. */
	oid_fanout_count = 0;
	for (i = 0; i < 256; i++) {
		while (oid_fanout_count < git_vector_length(&w->commits) &&
		       (packed_commit = (struct packed_commit *)git_vector_get(&w->commits, oid_fanout_count)) &&
		       packed_commit->sha1.id[0] <= i)
			++oid_fanout_count;
		oid_fanout[i] = htonl(oid_fanout_count);
	}

	/* Fill the OID Lookup table. */
	git_vector_foreach (&w->commits, i, packed_commit) {
		error = git_str_put(&oid_lookup,
			(const char *)&packed_commit->sha1, sizeof(git_oid));
		if (error < 0)
			goto cleanup;
	}

	/* Fill the Commit Data and Extra Edge List tables. */
	extra_edge_list_count = 0;
	git_vector_foreach (&w->commits, i, packed_commit) {
		uint64_t commit_time;
		uint32_t generation;
		uint32_t word;
		size_t *packed_index;
		unsigned int parentcount = (unsigned int)git_array_size(packed_commit->parents);

		error = git_str_put(&commit_data,
				(const char *)&packed_commit->tree_oid,
				sizeof(git_oid));
		if (error < 0)
			goto cleanup;

		if (parentcount == 0) {
			word = htonl(GIT_COMMIT_GRAPH_MISSING_PARENT);
		} else {
			packed_index = git_array_get(packed_commit->parent_indices, 0);
			word = htonl((uint32_t)*packed_index);
		}
		error = git_str_put(&commit_data, (const char *)&word, sizeof(word));
		if (error < 0)
			goto cleanup;

		if (parentcount < 2) {
			word = htonl(GIT_COMMIT_GRAPH_MISSING_PARENT);
		} else if (parentcount == 2) {
			packed_index = git_array_get(packed_commit->parent_indices, 1);
			word = htonl((uint32_t)*packed_index);
		} else {
			word = htonl(0x80000000u | extra_edge_list_count);
		}
		error = git_str_put(&commit_data, (const char *)&word, sizeof(word));
		if (error < 0)
			goto cleanup;

		if (parentcount > 2) {
			unsigned int parent_i;
			for (parent_i = 1; parent_i < parentcount; ++parent_i) {
				packed_index = git_array_get(
					packed_commit->parent_indices, parent_i);
				word = htonl((uint32_t)(*packed_index | (parent_i + 1 == parentcount ? 0x80000000u : 0)));

				error = git_str_put(&extra_edge_list,
						(const char *)&word,
						sizeof(word));
				if (error < 0)
					goto cleanup;
			}
			extra_edge_list_count += parentcount - 1;
		}

		generation = packed_commit->generation;
		commit_time = (uint64_t)packed_commit->commit_time;
		if (generation > GIT_COMMIT_GRAPH_GENERATION_NUMBER_MAX)
			generation = GIT_COMMIT_GRAPH_GENERATION_NUMBER_MAX;
		word = ntohl((uint32_t)((generation << 2) | (((uint32_t)(commit_time >> 32)) & 0x3) ));
		error = git_str_put(&commit_data, (const char *)&word, sizeof(word));
		if (error < 0)
			goto cleanup;
		word = ntohl((uint32_t)(commit_time & 0xfffffffful));
		error = git_str_put(&commit_data, (const char *)&word, sizeof(word));
		if (error < 0)
			goto cleanup;
	}

	/* Write the header. */
	hdr.chunks = 3;
	if (git_str_len(&extra_edge_list) > 0)
		hdr.chunks++;
	error = write_cb((const char *)&hdr, sizeof(hdr), cb_data);
	if (error < 0)
		goto cleanup;

	/* Write the chunk headers. */
	offset = sizeof(hdr) + (hdr.chunks + 1) * 12;
	error = write_chunk_header(COMMIT_GRAPH_OID_FANOUT_ID, offset, write_cb, cb_data);
	if (error < 0)
		goto cleanup;
	offset += sizeof(oid_fanout);
	error = write_chunk_header(COMMIT_GRAPH_OID_LOOKUP_ID, offset, write_cb, cb_data);
	if (error < 0)
		goto cleanup;
	offset += git_str_len(&oid_lookup);
	error = write_chunk_header(COMMIT_GRAPH_COMMIT_DATA_ID, offset, write_cb, cb_data);
	if (error < 0)
		goto cleanup;
	offset += git_str_len(&commit_data);
	if (git_str_len(&extra_edge_list) > 0) {
		error = write_chunk_header(
				COMMIT_GRAPH_EXTRA_EDGE_LIST_ID, offset, write_cb, cb_data);
		if (error < 0)
			goto cleanup;
		offset += git_str_len(&extra_edge_list);
	}
	error = write_chunk_header(0, offset, write_cb, cb_data);
	if (error < 0)
		goto cleanup;

	/* Write all the chunks. */
	error = write_cb((const char *)oid_fanout, sizeof(oid_fanout), cb_data);
	if (error < 0)
		goto cleanup;
	error = write_cb(git_str_cstr(&oid_lookup), git_str_len(&oid_lookup), cb_data);
	if (error < 0)
		goto cleanup;
	error = write_cb(git_str_cstr(&commit_data), git_str_len(&commit_data), cb_data);
	if (error < 0)
		goto cleanup;
	error = write_cb(git_str_cstr(&extra_edge_list), git_str_len(&extra_edge_list), cb_data);
	if (error < 0)
		goto cleanup;

	/* Finalize the checksum and write the trailer. */
	error = git_hash_final(checksum, &ctx);
	if (error < 0)
		goto cleanup;
	error = write_cb((char *)checksum, checksum_size, cb_data);
	if (error < 0)
		goto cleanup;

cleanup:
	git_str_dispose(&oid_lookup);
	git_str_dispose(&commit_data);
	git_str_dispose(&extra_edge_list);
	git_hash_ctx_cleanup(&ctx);
	return error;
}


// Source: commit_graph.c
// Lines 966-1155
