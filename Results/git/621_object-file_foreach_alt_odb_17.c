int foreach_alt_odb(alt_odb_fn fn, void *cb)
{
	struct object_directory *ent;
	int r = 0;

	prepare_alt_odb(the_repository);
	for (ent = the_repository->objects->odb->next; ent; ent = ent->next) {
		r = fn(ent, cb);
		if (r)
			break;
	}
	return r;
}


// Source: object-file.c
// Lines 875-887
