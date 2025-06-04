static int check_and_freshen_nonlocal(const struct object_id *oid, int freshen)
{
	struct object_directory *odb;

	prepare_alt_odb(the_repository);
	for (odb = the_repository->objects->odb->next; odb; odb = odb->next) {
		if (check_and_freshen_odb(odb, oid, freshen))
			return 1;
	}
	return 0;
}


// Source: object-file.c
// Lines 936-946
