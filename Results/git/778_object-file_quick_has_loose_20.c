static int quick_has_loose(struct repository *r,
			   const struct object_id *oid)
{
	struct object_directory *odb;

	prepare_alt_odb(r);
	for (odb = r->objects->odb; odb; odb = odb->next) {
		if (oidtree_contains(odb_loose_cache(odb, oid), oid))
			return 1;
	}
	return 0;
}


// Source: object-file.c
// Lines 1137-1148
