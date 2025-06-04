static void prepare_packed_git(struct repository *r)
{
	struct object_directory *odb;

	if (r->objects->packed_git_initialized)
		return;

	prepare_alt_odb(r);
	for (odb = r->objects->odb; odb; odb = odb->next) {
		int local = (odb == r->objects->odb);
		prepare_multi_pack_index_one(r, odb->path, local);
		prepare_packed_git_one(r, odb->path, local);
	}
	rearrange_packed_git(r);

	prepare_packed_git_mru(r);
	r->objects->packed_git_initialized = 1;
}


// Source: packfile.c
// Lines 983-1000
