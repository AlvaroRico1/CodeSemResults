int for_each_packed_object(each_packed_object_fn cb, void *data,
			   enum for_each_object_flags flags)
{
	struct packed_git *p;
	int r = 0;
	int pack_errors = 0;

	prepare_packed_git(the_repository);
	for (p = get_all_packs(the_repository); p; p = p->next) {
		if ((flags & FOR_EACH_OBJECT_LOCAL_ONLY) && !p->pack_local)
			continue;
		if ((flags & FOR_EACH_OBJECT_PROMISOR_ONLY) &&
		    !p->pack_promisor)
			continue;
		if ((flags & FOR_EACH_OBJECT_SKIP_IN_CORE_KEPT_PACKS) &&
		    p->pack_keep_in_core)
			continue;
		if ((flags & FOR_EACH_OBJECT_SKIP_ON_DISK_KEPT_PACKS) &&
		    p->pack_keep)
			continue;
		if (open_pack_index(p)) {
			pack_errors = 1;
			continue;
		}
		r = for_each_object_in_pack(p, cb, data, flags);
		if (r)
			break;
	}
	return r ? r : pack_errors;
}


// Source: packfile.c
// Lines 2179-2208
