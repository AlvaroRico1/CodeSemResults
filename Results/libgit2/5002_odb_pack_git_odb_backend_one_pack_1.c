int git_odb_backend_one_pack(git_odb_backend **backend_out, const char *idx)
{
	struct pack_backend *backend = NULL;
	struct git_pack_file *packfile = NULL;

	if (pack_backend__alloc(&backend, 1) < 0)
		return -1;

	if (git_mwindow_get_pack(&packfile, idx) < 0 ||
		git_vector_insert(&backend->packs, packfile) < 0)
	{
		pack_backend__free((git_odb_backend *)backend);
		return -1;
	}

	*backend_out = (git_odb_backend *)backend;
	return 0;
}


// Source: odb_pack.c
// Lines 876-893
