void reprepare_packed_git(struct repository *r)
{
	struct object_directory *odb;

	obj_read_lock();
	for (odb = r->objects->odb; odb; odb = odb->next)
		odb_clear_loose_cache(odb);

	r->objects->approximate_object_count_valid = 0;
	r->objects->packed_git_initialized = 0;
	prepare_packed_git(r);
	obj_read_unlock();
}


// Source: packfile.c
// Lines 1002-1014
