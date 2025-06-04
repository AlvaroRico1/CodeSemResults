void git_indexer_free(git_indexer *idx)
{
	const git_oid *key;
	git_oid *value;
	size_t iter;

	if (idx == NULL)
		return;

	if (idx->have_stream)
		git_packfile_stream_dispose(&idx->stream);

	git_vector_free_deep(&idx->objects);

	if (idx->pack->idx_cache) {
		struct git_pack_entry *pentry;
		git_oidmap_foreach_value(idx->pack->idx_cache, pentry, {
			git__free(pentry);
		});

		git_oidmap_free(idx->pack->idx_cache);
	}


// Source: indexer.c
// Lines 1377-1398
